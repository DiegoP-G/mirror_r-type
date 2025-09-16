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
    
    void createTextures() {
        // Create colored textures for game objects
        SDL_Texture* birdTexture = g_graphics->createColorTexture(32, 32, 255, 255, 0); // Yellow bird
        SDL_Texture* pipeTexture = g_graphics->createColorTexture(80, 400, 0, 255, 0);   // Green pipes
        
        if (birdTexture) g_graphics->storeTexture("bird", birdTexture);
        if (pipeTexture) g_graphics->storeTexture("pipe", pipeTexture);
    }
    
    void createBird() {
        auto& birdEntity = entityManager.createEntity();
        
        birdEntity.addComponent<TransformComponent>(100.0f, 300.0f);
        birdEntity.addComponent<VelocityComponent>(0.0f, 0.0f);
        birdEntity.addComponent<SpriteComponent>(32, 32, 255, 255, 0); // Yellow
        birdEntity.addComponent<ColliderComponent>(32.0f, 32.0f);
        birdEntity.addComponent<JumpComponent>(-400.0f); // Generic jump instead of BirdComponent
        birdEntity.addComponent<GravityComponent>(1200.0f, 800.0f);
        birdEntity.addComponent<InputComponent>();
        
        bird = &birdEntity;
    }
    
    void spawnPipe() {
        float pipeHeight = pipeHeightDist(rng);
        
        // Bottom pipe
        auto& bottomPipe = entityManager.createEntity();
        bottomPipe.addComponent<TransformComponent>(800.0f, pipeHeight);
        bottomPipe.addComponent<VelocityComponent>(PIPE_SPEED, 0.0f);
        bottomPipe.addComponent<SpriteComponent>(80, static_cast<int>(600 - pipeHeight), 0, 255, 0); // Green
        bottomPipe.addComponent<ColliderComponent>(80.0f, 600 - pipeHeight);
        // Remove PipeComponent - not needed with generic systems
        
        // Top pipe
        float topPipeHeight = pipeHeight - PIPE_GAP;
        auto& topPipe = entityManager.createEntity();
        topPipe.addComponent<TransformComponent>(800.0f, 0.0f);
        topPipe.addComponent<VelocityComponent>(PIPE_SPEED, 0.0f);
        topPipe.addComponent<SpriteComponent>(80, static_cast<int>(topPipeHeight), 0, 255, 0); // Green
        topPipe.addComponent<ColliderComponent>(80.0f, topPipeHeight);
        // Remove PipeComponent - not needed with generic systems
    }
    
    void handleEvents() {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
            
            // Handle input generically through InputComponent
            if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_SPACE) {
                if (!gameStarted) {
                    gameStarted = true;
                } else if (gameOver) {
                    restart();
                } else {
                    // Set fire input for jumping entities
                    auto entities = entityManager.getEntitiesWithComponents<InputComponent>();
                    for (auto& entity : entities) {
                        entity->getComponent<InputComponent>().fire = true;
                    }
                }
            }
        }
    }
    
    void update(float deltaTime) {
        if (!gameStarted || gameOver) return;
        
        // Spawn pipes
        pipeSpawnTimer -= deltaTime;
        if (pipeSpawnTimer <= 0.0f) {
            spawnPipe();
            pipeSpawnTimer = PIPE_SPAWN_INTERVAL;
        }
        
        // Update systems - all generic now!
        jumpSystem.update(entityManager, deltaTime);           // Generic jump
        gravitySystem.update(entityManager, deltaTime);        // Generic gravity  
        movementSystem.update(entityManager, deltaTime);       // Generic movement
        boundarySystem.update(entityManager, deltaTime);       // Generic boundaries
        cleanupSystem.update(entityManager, deltaTime);        // Generic cleanup
        collisionSystem.update(entityManager);                 // Generic collisions
        
        // Check game over - simplified since we use generic boundary system
        // The boundary system will destroy entities that go out of bounds
        if (bird && !bird->isActive()) {
            gameOver = true;
        }
        
        // Update score (simple scoring based on time survived)
        static float scoreTimer = 0.0f;
        scoreTimer += deltaTime;
        if (scoreTimer >= 1.0f) {
            score++;
            scoreTimer = 0.0f;
        }
        
        // Clean up destroyed entities
        entityManager.refresh();
    }
    
    void render() {
        g_graphics->clear();
        
        // Render all entities
        renderSystem.update(entityManager);
        
        // Render UI
        if (!gameStarted) {
            g_graphics->drawText("Press SPACE to start", 300, 250);
        }
        
        if (gameOver) {
            g_graphics->drawText("Game Over! Press SPACE to restart", 250, 250);
        }
        
        // Render score
        std::string scoreText = "Score: " + std::to_string(score);
        g_graphics->drawText(scoreText, 10, 10);
        
        g_graphics->present();
    }
    
    void restart() {
        // Clear all entities
        entityManager = EntityManager();
        
        // Reset game state
        score = 0;
        gameOver = false;
        gameStarted = false;
        pipeSpawnTimer = PIPE_SPAWN_INTERVAL;
        
        // Create new bird
        createBird();
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
    
    void cleanup() {
        if (g_graphics) {
            delete g_graphics;
            g_graphics = nullptr;
        }
    }
};

int main() {
    FlappyBirdGame game;
    game.run();
    return 0;
}