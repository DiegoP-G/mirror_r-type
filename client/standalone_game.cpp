#include "../ecs/ecs.hpp"
#include "../ecs/components.hpp"
#include "../ecs/systems.hpp"
#include "graphics.hpp"
#include <iostream>
#include <random>

class FlappyBirdGame {
private:
    EntityManager entityManager;
    
    // Systems
    MovementSystem movementSystem;
    RenderSystem renderSystem;
    CollisionSystem collisionSystem;
    GravitySystem gravitySystem;
    JumpSystem jumpSystem;                    // Generic instead of BirdSystem
    BoundarySystem boundarySystem;            // Generic boundary checking
    OffscreenCleanupSystem cleanupSystem;     // Generic cleanup
    InputSystem inputSystem;                  // Use existing generic input system
    
    bool running = false;
    float pipeSpawnTimer = 0.0f;
    const float PIPE_SPAWN_INTERVAL = 2.0f;
    const float PIPE_SPEED = -200.0f;
    const float PIPE_GAP = 200.0f;
    
    Entity* bird = nullptr;
    int score = 0;
    bool gameStarted = false;
    bool gameOver = false;
    
    std::mt19937 rng;
    std::uniform_real_distribution<float> pipeHeightDist;
    
public:
    FlappyBirdGame() : rng(std::random_device{}()), pipeHeightDist(100.0f, 400.0f) {}
    
    bool init() {
        // Initialize graphics
        g_graphics = new GraphicsManager();
        if (!g_graphics->init("Flappy Bird", 800, 600)) {
            std::cerr << "Failed to initialize graphics!" << std::endl;
            return false;
        }
        
        // Create textures
        createTextures();
        
        // Create bird
        createBird();
        
        running = true;
        
        std::cout << "Flappy Bird initialized!" << std::endl;
        return true;
    }
    
    void run() {
        if (!init()) {
            return;
        }
        
        const float TARGET_FPS = 60.0f;
        const float FRAME_TIME = 1.0f / TARGET_FPS;
        
        Uint32 lastTime = SDL_GetTicks();
        float accumulator = 0.0f;
        
        while (running) {
            Uint32 currentTime = SDL_GetTicks();
            float deltaTime = (currentTime - lastTime) / 1000.0f;
            lastTime = currentTime;
            
            // Cap delta time to prevent large jumps
            if (deltaTime > 0.05f) {
                deltaTime = 0.05f;
            }
            
            accumulator += deltaTime;
            
            handleEvents();
            
            // Fixed timestep update
            while (accumulator >= FRAME_TIME) {
                update(FRAME_TIME);
                accumulator -= FRAME_TIME;
            }
            
            render();
        }
        
        cleanup();
    }
    
    // ...rest of existing methods...
};

int main() {
    FlappyBirdGame game;
    game.run();
    return 0;
}