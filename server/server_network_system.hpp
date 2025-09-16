#pragma once
#include "../ecs/ecs.hpp"
#include "../ecs/components.hpp"
#include "../ecs/network_components.hpp"
#include "../network/network_messages.hpp"
#include "../network/udp_socket.hpp"
#include <map>
#include <queue>
#include <random>
#include <iostream>
#include <chrono>

// Helper function to get current timestamp
inline float getCurrentTime() {
    static auto start = std::chrono::high_resolution_clock::now();
    auto now = std::chrono::high_resolution_clock::now();
    return std::chrono::duration<float>(now - start).count();
}
struct ClientInfo {
    struct sockaddr_in address;
    uint32_t playerID;
    float lastHeartbeat;
    
    ClientInfo(const struct sockaddr_in& addr, uint32_t id) 
        : address(addr), playerID(id), lastHeartbeat(getCurrentTime()) {}
};

struct PipeCreateData {
    float x, y, width, height;
    bool isBottomPipe;
};


class ServerNetworkSystem {
private:
    UDPSocket socket;
    std::map<uint32_t, ClientInfo> clients;
    std::map<uint32_t, Entity*> networkedEntities;
    std::queue<NetworkMessage> incomingMessages;
    uint32_t nextEntityID = 1;
    uint32_t nextPlayerID = 1;
    
    // Game state
    float pipeSpawnTimer = 0.0f;
    const float PIPE_SPAWN_INTERVAL = 2.0f;
    const float PIPE_SPEED = -200.0f;
    const float PIPE_GAP = 200.0f;
    std::mt19937 rng;
    std::uniform_real_distribution<float> pipeHeightDist;
    
public:
    ServerNetworkSystem() : rng(std::random_device{}()), pipeHeightDist(100.0f, 400.0f) {}
    
    bool initialize(int port) {
        return socket.createServer(port);
    }
    
    void update(EntityManager& entityManager, float deltaTime) {
        receiveMessages(entityManager);
        processMessages(entityManager);
        updateGameLogic(entityManager, deltaTime);
        synchronizeEntities(entityManager);
        checkClientTimeouts();
    }
    
    void receiveMessages(EntityManager& entityManager) {
        NetworkMessage message;
        struct sockaddr_in senderAddr;
        
        while (socket.receiveMessage(message, senderAddr)) {
            handleIncomingMessage(message, senderAddr, entityManager);
        }
    }
    
    void handleIncomingMessage(const NetworkMessage& message, const struct sockaddr_in& senderAddr, EntityManager& entityManager) {
        switch (message.header.type) {
            case MessageType::PLAYER_JOIN:
                handlePlayerJoin(senderAddr, entityManager);
                break;
            case MessageType::PLAYER_INPUT:
                handlePlayerInput(message, senderAddr);
                break;
            default:
                break;
        }
    }
    
    void handlePlayerJoin(const struct sockaddr_in& clientAddr, EntityManager& entityManager) {
        uint32_t playerID = nextPlayerID++;
        clients.emplace(playerID, ClientInfo(clientAddr, playerID));
        
        // Create player entity automatically
        uint32_t playerEntityID = createPlayerEntity(entityManager, playerID);
        
        // Send both player ID and entity ID back to client
        NetworkMessage response(MessageType::PLAYER_JOIN, playerEntityID, getCurrentTime());
        
        struct PlayerJoinResponse {
            uint32_t playerID;
            uint32_t entityID;
        } joinData;
        
        joinData.playerID = playerID;
        joinData.entityID = playerEntityID;
        response.setData(joinData);
        
        socket.sendMessage(response, &clientAddr);
        
        std::cout << "Player " << playerID << " joined with entity " << playerEntityID << std::endl;
    }
    
    void handlePlayerInput(const NetworkMessage& message, const struct sockaddr_in& senderAddr) {
        // Find client
        for (auto& [playerID, client] : clients) {
            if (memcmp(&client.address, &senderAddr, sizeof(sockaddr_in)) == 0) {
                client.lastHeartbeat = getCurrentTime();
                
                // Apply input to player entity
                auto it = networkedEntities.find(message.header.entityID);
                if (it != networkedEntities.end() && it->second->hasComponent<InputComponent>()) {
                    PlayerInputData inputData = message.getData<PlayerInputData>();
                    auto& input = it->second->getComponent<InputComponent>();
                    input.fire = inputData.jump;
                }
                break;
            }
        }
    }
    
    void processMessages(EntityManager& entityManager) {
        // Process any queued messages
    }
    
    void updateGameLogic(EntityManager& entityManager, float deltaTime) {
        // Spawn pipes on server
        pipeSpawnTimer -= deltaTime;
        if (pipeSpawnTimer <= 0.0f) {
            spawnPipe(entityManager);
            pipeSpawnTimer = PIPE_SPAWN_INTERVAL;
        }
    }
    
    void spawnPipe(EntityManager& entityManager) {
        float pipeHeight = pipeHeightDist(rng);
        
        // Bottom pipe
        auto& bottomPipe = entityManager.createEntity();
        uint32_t bottomPipeID = nextEntityID++;
        
        bottomPipe.addComponent<TransformComponent>(800.0f, pipeHeight);
        bottomPipe.addComponent<VelocityComponent>(PIPE_SPEED, 0.0f);
        bottomPipe.addComponent<ColliderComponent>(80.0f, 600 - pipeHeight);
        bottomPipe.addComponent<NetworkComponent>(bottomPipeID, true);
        bottomPipe.addComponent<ServerAuthorityComponent>(true);
        
        networkedEntities[bottomPipeID] = &bottomPipe;
        
        // Top pipe
        float topPipeHeight = pipeHeight - PIPE_GAP;
        auto& topPipe = entityManager.createEntity();
        uint32_t topPipeID = nextEntityID++;
        
        topPipe.addComponent<TransformComponent>(800.0f, 0.0f);
        topPipe.addComponent<VelocityComponent>(PIPE_SPEED, 0.0f);
        topPipe.addComponent<ColliderComponent>(80.0f, topPipeHeight);
        topPipe.addComponent<NetworkComponent>(topPipeID, true);
        topPipe.addComponent<ServerAuthorityComponent>(true);
        
        networkedEntities[topPipeID] = &topPipe;
        
        // Send pipe creation to all clients
        broadcastPipeCreation(bottomPipeID, bottomPipe, true);
        broadcastPipeCreation(topPipeID, topPipe, false);
    }
    
    void broadcastPipeCreation(uint32_t pipeID, Entity& pipe, bool isBottom) {
        auto& transform = pipe.getComponent<TransformComponent>();
        auto& collider = pipe.getComponent<ColliderComponent>();
        
        NetworkMessage message(MessageType::ENTITY_CREATE, pipeID, getCurrentTime());
        
        PipeCreateData pipeData;
        pipeData.x = transform.position.x;
        pipeData.y = transform.position.y;
        pipeData.width = collider.hitbox.w;
        pipeData.height = collider.hitbox.h;
        pipeData.isBottomPipe = isBottom;
        
        message.setData(pipeData);
        
        broadcastToAllClients(message);
    }
    
    void synchronizeEntities(EntityManager& entityManager) {
        auto entities = entityManager.getEntitiesWithComponents<NetworkComponent, TransformComponent>();
        
        for (auto& entity : entities) {
            auto& network = entity->getComponent<NetworkComponent>();
            auto& transform = entity->getComponent<TransformComponent>();
            
            if (network.isDirty || (entity->hasComponent<ServerAuthorityComponent>() && 
                getCurrentTime() - entity->getComponent<ServerAuthorityComponent>().lastSyncTime > 0.1f)) {
                // Send entity update
                NetworkMessage message(MessageType::ENTITY_UPDATE, network.networkID, getCurrentTime());
                
                EntityUpdateData updateData;
                updateData.x = transform.position.x;
                updateData.y = transform.position.y;
                
                if (entity->hasComponent<VelocityComponent>()) {
                    auto& velocity = entity->getComponent<VelocityComponent>();
                    updateData.velocityX = velocity.velocity.x;
                    updateData.velocityY = velocity.velocity.y;
                }
                
                message.setData(updateData);
                broadcastToAllClients(message);
                
                network.isDirty = false;
                if (entity->hasComponent<ServerAuthorityComponent>()) {
                    entity->getComponent<ServerAuthorityComponent>().lastSyncTime = getCurrentTime();
                }
            }
        }
    }
    
    void handleCollision(Entity* entityA, Entity* entityB) {
        uint32_t idA = 0, idB = 0;
        
        if (entityA->hasComponent<NetworkComponent>()) {
            idA = entityA->getComponent<NetworkComponent>().networkID;
        }
        if (entityB->hasComponent<NetworkComponent>()) {
            idB = entityB->getComponent<NetworkComponent>().networkID;
        }
        
        // Broadcast collision event
        NetworkMessage message(MessageType::COLLISION_EVENT, 0, getCurrentTime());
        CollisionEventData collisionData(idA, idB);
        message.setData(collisionData);
        
        broadcastToAllClients(message);
    }
    
    void broadcastToAllClients(const NetworkMessage& message) {
        for (const auto& [playerID, client] : clients) {
            socket.sendMessage(message, &client.address);
        }
    }
    
    void checkClientTimeouts() {
        float currentTime = getCurrentTime();
        auto it = clients.begin();
        
        while (it != clients.end()) {
            if (currentTime - it->second.lastHeartbeat > 10.0f) { // 10 second timeout
                std::cout << "Player " << it->first << " timed out" << std::endl;
                it = clients.erase(it);
            } else {
                ++it;
            }
        }
    }
    
    uint32_t createPlayerEntity(EntityManager& entityManager, uint32_t playerID) {
        auto& player = entityManager.createEntity();
        uint32_t entityID = nextEntityID++;
        
        player.addComponent<TransformComponent>(100.0f, 300.0f);
        player.addComponent<VelocityComponent>(0.0f, 0.0f); // Player should not move horizontally
        player.addComponent<JumpComponent>(-400.0f);
        player.addComponent<GravityComponent>(1200.0f, 800.0f);
        player.addComponent<InputComponent>();
        player.addComponent<NetworkComponent>(entityID, true);
        player.addComponent<ServerAuthorityComponent>(true);
        player.addComponent<PlayerComponent>(playerID, false);
        
        networkedEntities[entityID] = &player;
        
        return entityID;
    }
};