#include "../ecs/ecs.hpp"
#include "../ecs/components.hpp"
#include "../ecs/systems.hpp"
#include "../ecs/network_components.hpp"
#include "../server/server_network_system.hpp"
#include <iostream>
#include <chrono>
#include <thread>

class FlappyBirdServer {
private:
    EntityManager entityManager;
    
    // Systems
    MovementSystem movementSystem;
    GravitySystem gravitySystem;
    JumpSystem jumpSystem;
    BoundarySystem boundarySystem;
    OffscreenCleanupSystem cleanupSystem;
    ServerNetworkSystem networkSystem;
    
    bool running = false;
    
public:
    bool init(int port) {
        if (!networkSystem.initialize(port)) {
            std::cerr << "Failed to initialize server on port " << port << std::endl;
            return false;
        }
        
        running = true;
        
        std::cout << "Flappy Bird Server started on port " << port << std::endl;
        return true;
    }
    
    void run() {
        if (!init(12345)) {
            return;
        }
        
        const float TARGET_FPS = 60.0f;
        const float FRAME_TIME = 1.0f / TARGET_FPS;
        
        auto lastTime = std::chrono::high_resolution_clock::now();
        float accumulator = 0.0f;
        
        while (running) {
            auto currentTime = std::chrono::high_resolution_clock::now();
            float deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
            lastTime = currentTime;
            
            // Cap delta time to prevent large jumps
            if (deltaTime > 0.05f) {
                deltaTime = 0.05f;
            }
            
            accumulator += deltaTime;
            
            // Fixed timestep update
            while (accumulator >= FRAME_TIME) {
                update(FRAME_TIME);
                accumulator -= FRAME_TIME;
            }
            
            // Small sleep to prevent 100% CPU usage
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        
        cleanup();
    }
    
    void update(float deltaTime) {
        // Network updates first
        networkSystem.update(entityManager, deltaTime);
        
        // Game logic systems
        jumpSystem.update(entityManager, deltaTime);
        gravitySystem.update(entityManager, deltaTime);
        
        // Custom movement system that only moves non-player entities
        updateMovement(deltaTime);
        
        boundarySystem.update(entityManager, deltaTime);
        cleanupSystem.update(entityManager, deltaTime);
        
        // Custom collision system that notifies network
        updateCollisions();
        
        // Clean up destroyed entities
        entityManager.refresh();
    }
    
    void updateMovement(float deltaTime) {
        // Only move entities that are NOT players (pipes, projectiles, etc.)
        auto entities = entityManager.getEntitiesWithComponents<TransformComponent, VelocityComponent>();
        
        for (auto& entity : entities) {
            // Skip player entities
            if (entity->hasComponent<PlayerComponent>()) {
                // Players should only move vertically due to gravity/jump
                auto& transform = entity->getComponent<TransformComponent>();
                auto& velocity = entity->getComponent<VelocityComponent>();
                
                // Only apply vertical movement for players
                transform.position.y += velocity.velocity.y * deltaTime;
                // Keep X position fixed for players
                velocity.velocity.x = 0.0f;
            } else {
                // Regular movement for non-player entities (pipes, etc.)
                auto& transform = entity->getComponent<TransformComponent>();
                auto& velocity = entity->getComponent<VelocityComponent>();
                
                transform.position += velocity.velocity * deltaTime;
            }
        }
    }
    
    void updateCollisions() {
        auto entities = entityManager.getEntitiesWithComponents<TransformComponent, ColliderComponent>();
        
        for (size_t i = 0; i < entities.size(); i++) {
            if (!entities[i]->isActive()) continue;
            
            auto& transform1 = entities[i]->getComponent<TransformComponent>();
            auto& collider1 = entities[i]->getComponent<ColliderComponent>();
            
            collider1.hitbox.x = transform1.position.x;
            collider1.hitbox.y = transform1.position.y;
            
            for (size_t j = i + 1; j < entities.size(); j++) {
                if (!entities[j]->isActive()) continue;
                
                auto& transform2 = entities[j]->getComponent<TransformComponent>();
                auto& collider2 = entities[j]->getComponent<ColliderComponent>();
                
                collider2.hitbox.x = transform2.position.x;
                collider2.hitbox.y = transform2.position.y;
                
                if (collider1.hitbox.intersects(collider2.hitbox)) {
                    handleCollision(entities[i], entities[j]);
                }
            }
        }
    }
    
    void handleCollision(Entity* a, Entity* b) {
        // Check for player-pipe collision
        bool aIsPlayer = a->hasComponent<PlayerComponent>();
        bool bIsPlayer = b->hasComponent<PlayerComponent>();
        
        if (aIsPlayer || bIsPlayer) {
            Entity* player = aIsPlayer ? a : b;
            
            // Destroy player on collision
            player->destroy();
            
            // Notify clients via network
            networkSystem.handleCollision(a, b);
            
            std::cout << "Player collision detected!" << std::endl;
        }
    }
    
    void cleanup() {
        std::cout << "Server shutting down..." << std::endl;
    }
};

int main(int argc, char* argv[]) {
    FlappyBirdServer server;
    server.run();
    return 0;
}