#pragma once
#include "../ecs/ecs.hpp"
#include "../ecs/components.hpp"
#include "../ecs/network_components.hpp"
#include "../network/network_messages.hpp"
#include "../network/udp_socket.hpp"
#include <map>
#include <SDL2/SDL.h>
#include <iostream>
#include <chrono>

// Helper function to get current timestamp (same as server)
inline float getCurrentTime() {
    static auto start = std::chrono::high_resolution_clock::now();
    auto now = std::chrono::high_resolution_clock::now();
    return std::chrono::duration<float>(now - start).count();
}

struct PipeCreateData {
    float x, y, width, height;
    bool isBottomPipe;
};

// Import from network_messages.hpp
struct PlayerJoinResponse {
    uint32_t playerID;
    uint32_t entityID;
};

class ClientNetworkSystem {
private:
    UDPSocket socket;
    std::map<uint32_t, Entity*> networkedEntities;
    uint32_t localPlayerID = 0;
    uint32_t localPlayerEntityID = 0;
    bool connected = false;
    float lastInputSent = 0.0f;
    
public:
    bool connectToServer(const std::string& serverIP, int serverPort) {
        if (!socket.createClient(serverIP, serverPort)) {
            return false;
        }
        
        // Send join request
        NetworkMessage joinMessage(MessageType::PLAYER_JOIN, 0, SDL_GetTicks() / 1000.0f);
        socket.sendMessage(joinMessage);
        
        return true;
    }
    
    void update(EntityManager& entityManager, float deltaTime) {
        receiveMessages(entityManager);
        sendInputToServer(entityManager);
        updateClientPrediction(entityManager, deltaTime);
    }
    
    void receiveMessages(EntityManager& entityManager) {
        NetworkMessage message;
        struct sockaddr_in senderAddr;
        
        while (socket.receiveMessage(message, senderAddr)) {
            handleMessage(entityManager, message);
        }
    }
    
    void handleMessage(EntityManager& entityManager, const NetworkMessage& message) {
        switch (message.header.type) {
            case MessageType::PLAYER_JOIN:
                handlePlayerJoin(message);
                break;
            case MessageType::ENTITY_CREATE:
                handleEntityCreate(entityManager, message);
                break;
            case MessageType::ENTITY_UPDATE:
                handleEntityUpdate(message);
                break;
            case MessageType::COLLISION_EVENT:
                handleCollisionEvent(message);
                break;
            default:
                break;
        }
    }
    
    void handlePlayerJoin(const NetworkMessage& message) {
        PlayerJoinResponse joinData = message.getData<PlayerJoinResponse>();
        localPlayerID = joinData.playerID;
        localPlayerEntityID = joinData.entityID;
        connected = true;
        std::cout << "Connected to server as player " << localPlayerID << " with entity " << localPlayerEntityID << std::endl;
    }
    
    void handleEntityCreate(EntityManager& entityManager, const NetworkMessage& message) {
        uint32_t entityID = message.header.entityID;
        
        // Create entity based on message data
        auto& entity = entityManager.createEntity();
        
        PipeCreateData pipeData = message.getData<PipeCreateData>();
        
        entity.addComponent<TransformComponent>(pipeData.x, pipeData.y);
        entity.addComponent<VelocityComponent>(-200.0f, 0.0f);
        entity.addComponent<ColliderComponent>(pipeData.width, pipeData.height);
        entity.addComponent<NetworkComponent>(entityID, true);
        
        networkedEntities[entityID] = &entity;
    }
    
    void handleEntityUpdate(const NetworkMessage& message) {
        uint32_t entityID = message.header.entityID;
        auto it = networkedEntities.find(entityID);
        
        if (it != networkedEntities.end()) {
            EntityUpdateData updateData = message.getData<EntityUpdateData>();
            
            auto& transform = it->second->getComponent<TransformComponent>();
            transform.position.x = updateData.x;
            transform.position.y = updateData.y;
            
            if (it->second->hasComponent<VelocityComponent>()) {
                auto& velocity = it->second->getComponent<VelocityComponent>();
                velocity.velocity.x = updateData.velocityX;
                velocity.velocity.y = updateData.velocityY;
            }
        }
    }
    
    void handleCollisionEvent(const NetworkMessage& message) {
        CollisionEventData collisionData = message.getData<CollisionEventData>();
        
        // Handle collision effects
        auto entityA = networkedEntities.find(collisionData.entityA);
        auto entityB = networkedEntities.find(collisionData.entityB);
        
        if (entityA != networkedEntities.end()) {
            if (entityA->second->hasComponent<PlayerComponent>()) {
                // Player collision - game over
                entityA->second->destroy();
            }
        }
        
        if (entityB != networkedEntities.end()) {
            if (entityB->second->hasComponent<PlayerComponent>()) {
                // Player collision - game over
                entityB->second->destroy();
            }
        }
    }
    
    void sendInputToServer(EntityManager& entityManager) {
        if (!connected || localPlayerEntityID == 0) return;
        
        float currentTime = getCurrentTime();
        
        auto entities = entityManager.getEntitiesWithComponents<InputComponent, PlayerComponent>();
        
        for (auto& entity : entities) {
            auto& input = entity->getComponent<InputComponent>();
            auto& player = entity->getComponent<PlayerComponent>();
            
            if (player.isLocalPlayer && (input.fire || currentTime - lastInputSent > 0.1f)) {
                NetworkMessage message(MessageType::PLAYER_INPUT, localPlayerEntityID, currentTime);
                
                PlayerInputData inputData(input.fire, currentTime);
                message.setData(inputData);
                
                socket.sendMessage(message);
                
                std::cout << "Sending input: jump=" << input.fire << " to entity " << localPlayerEntityID << std::endl;
                
                input.fire = false; // Reset after sending
                lastInputSent = currentTime;
            }
        }
    }
    
    void updateClientPrediction(EntityManager& entityManager, float deltaTime) {
        // Client-side prediction for local player only
        auto entities = entityManager.getEntitiesWithComponents<PlayerComponent, ClientPredictionComponent>();
        
        for (auto& entity : entities) {
            auto& player = entity->getComponent<PlayerComponent>();
            
            if (player.isLocalPlayer) {
                // Apply local prediction
                // This would run physics locally for smooth gameplay
            }
        }
    }
    
    uint32_t createLocalPlayer(EntityManager& entityManager) {
        auto& player = entityManager.createEntity();
        
        player.addComponent<TransformComponent>(100.0f, 300.0f);
        player.addComponent<VelocityComponent>(0.0f, 0.0f);
        player.addComponent<JumpComponent>(-400.0f);
        player.addComponent<GravityComponent>(1200.0f, 800.0f);
        player.addComponent<InputComponent>();
        player.addComponent<PlayerComponent>(localPlayerID, true);
        player.addComponent<ClientPredictionComponent>();
        
        localPlayerEntityID = 1; // This should be assigned by server
        networkedEntities[localPlayerEntityID] = &player;
        
        return localPlayerEntityID;
    }
    
    bool isConnected() const { return connected; }
    uint32_t getLocalPlayerID() const { return localPlayerID; }
};