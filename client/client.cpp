#include "../ecs/ecs.hpp"
#include "../ecs/components.hpp"
#include "../ecs/systems.hpp"
#include "../ecs/network_components.hpp"
#include "../client/client_network_system.hpp"
#include "../game/graphics.hpp"
#include <iostream>
#include <SDL2/SDL.h>

class FlappyBirdClient {
private:
    EntityManager entityManager;
    
    // Systems
    MovementSystem movementSystem;
    RenderSystem renderSystem;
    GravitySystem gravitySystem;
    JumpSystem jumpSystem;
    ClientNetworkSystem networkSystem;
    
    bool running = false;
    bool gameStarted = false;
    bool gameOver = false;
    int score = 0;
    
public:
    bool init(const std::string& serverIP = "127.0.0.1", int serverPort = 12345) {
        // Initialize graphics
        g_graphics = new GraphicsManager();
        if (!g_graphics->init("Flappy Bird Client", 800, 600)) {
            std::cerr << "Failed to initialize graphics!" << std::endl;
            return false;
        }
        
        // Create textures
        createTextures();
        
        // Connect to server
        if (!networkSystem.connectToServer(serverIP, serverPort)) {
            std::cerr << "Failed to connect to server!" << std::endl;
            return false;
        }
        
        running = true;
        
        std::cout << "Flappy Bird Client initialized!" << std::endl;
        return true;
    }
    
    void createTextures() {
        // Create colored textures for game objects
        SDL_Texture* birdTexture = g_graphics->createColorTexture(32, 32, 255, 255, 0); // Yellow bird
        SDL_Texture* pipeTexture = g_graphics->createColorTexture(80, 400, 0, 255, 0);   // Green pipes
        
        if (birdTexture) g_graphics->storeTexture("bird", birdTexture);
        if (pipeTexture) g_graphics->storeTexture("pipe", pipeTexture);
    }
    
    void handleEvents() {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
            
            if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_SPACE) {
                if (!gameStarted && networkSystem.isConnected()) {
                    gameStarted = true;
                    // Le joueur local est automatiquement créé par le serveur
                    createLocalPlayerFromServer();
                } else if (gameOver) {
                    restart();
                } else {
                    // Set fire input for local player
                    auto entities = entityManager.getEntitiesWithComponents<InputComponent, PlayerComponent>();
                    for (auto& entity : entities) {
                        auto& player = entity->getComponent<PlayerComponent>();
                        if (player.isLocalPlayer) {
                            entity->getComponent<InputComponent>().fire = true;
                            std::cout << "Jump input set!" << std::endl;
                        }
                    }
                }
            }
        }
    }
    
    void createLocalPlayerFromServer() {
        // Create local representation of the server's player entity
        auto& player = entityManager.createEntity();
        
        player.addComponent<TransformComponent>(100.0f, 300.0f);
        player.addComponent<VelocityComponent>(0.0f, 0.0f);
        player.addComponent<JumpComponent>(-400.0f);
        player.addComponent<GravityComponent>(1200.0f, 800.0f);
        player.addComponent<InputComponent>();
        player.addComponent<PlayerComponent>(networkSystem.getLocalPlayerID(), true);
        player.addComponent<ClientPredictionComponent>();
        
        std::cout << "Created local player representation" << std::endl;
    }
    
    void update(float deltaTime) {
        // Network updates first
        networkSystem.update(entityManager, deltaTime);
        
        if (!gameStarted || gameOver) return;
        
        // Client-side systems (for smooth gameplay)
        jumpSystem.update(entityManager, deltaTime);
        gravitySystem.update(entityManager, deltaTime);
        movementSystem.update(entityManager, deltaTime);
        
        // Check if local player is destroyed (game over)
        auto players = entityManager.getEntitiesWithComponents<PlayerComponent>();
        bool localPlayerAlive = false;
        for (auto& entity : players) {
            auto& player = entity->getComponent<PlayerComponent>();
            if (player.isLocalPlayer && entity->isActive()) {
                localPlayerAlive = true;
                break;
            }
        }
        
        if (!localPlayerAlive && gameStarted) {
            gameOver = true;
        }
        
        // Update score (simple time-based scoring)
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
        auto entities = entityManager.getEntitiesWithComponents<TransformComponent>();
        
        for (auto& entity : entities) {
            auto& transform = entity->getComponent<TransformComponent>();
            
            // Determine color based on entity type
            if (entity->hasComponent<PlayerComponent>()) {
                // Render player as yellow rectangle
                g_graphics->drawRect(
                    static_cast<int>(transform.position.x),
                    static_cast<int>(transform.position.y),
                    32, 32, 255, 255, 0, 255);
            } else if (entity->hasComponent<ColliderComponent>()) {
                // Render pipes as green rectangles
                auto& collider = entity->getComponent<ColliderComponent>();
                g_graphics->drawRect(
                    static_cast<int>(transform.position.x),
                    static_cast<int>(transform.position.y),
                    static_cast<int>(collider.hitbox.w),
                    static_cast<int>(collider.hitbox.h),
                    0, 255, 0, 255);
            }
        }
        
        // Render UI
        if (!networkSystem.isConnected()) {
            g_graphics->drawText("Connecting to server...", 300, 250);
        } else if (!gameStarted) {
            g_graphics->drawText("Press SPACE to start", 300, 250);
        }
        
        if (gameOver) {
            g_graphics->drawText("Game Over! Press SPACE to restart", 250, 250);
        }
        
        // Render score
        std::string scoreText = "Score: " + std::to_string(score);
        g_graphics->drawText(scoreText, 10, 10);
        
        // Render player ID
        if (networkSystem.isConnected()) {
            std::string playerText = "Player: " + std::to_string(networkSystem.getLocalPlayerID());
            g_graphics->drawText(playerText, 10, 30);
        }
        
        g_graphics->present();
    }
    
    void restart() {
        // Clear local entities
        entityManager = EntityManager();
        
        // Reset game state
        score = 0;
        gameOver = false;
        gameStarted = false;
        
        // Reconnect to server (simplified)
        // In a full implementation, you'd want to notify the server
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

int main(int argc, char* argv[]) {
    std::string serverIP = "127.0.0.1";
    
    if (argc > 1) {
        serverIP = argv[1];
    }
    
    FlappyBirdClient client;
    client.run();
    
    return 0;
}