#include "../ecs/components.hpp"
#include "../ecs/ecs.hpp"
#include "../ecs/systems.hpp"
#include <iostream>
#include <random>

class RTypeGame {
private:
  EntityManager entityManager;

  // Systems
  MovementSystem movementSystem;
  RenderSystem renderSystem;
  CollisionSystem collisionSystem;
  BoundarySystem boundarySystem;        // Generic boundary checking
  OffscreenCleanupSystem cleanupSystem; // Generic cleanup
  InputSystem inputSystem;              // Use existing generic input system

  Entity *player = nullptr;
  bool gameOver = false;

  bool running = false;

  int score = 0;

  std::mt19937 rng;

  const float ENEMY_SPEED = -200.0f;

public:
  RTypeGame() : rng(std::random_device{}()){};

  bool init() {
    // Initialize graphics
    g_graphics = new GraphicsManager();
    if (!g_graphics->init("R-Type", 800, 600)) {
      std::cerr << "Failed to initialize graphics!" << std::endl;
      return false;
    }

    // Create textures
    createTextures();

    // Create player
    createPlayer();

    running = true;

    std::cout << "R-Type initialized!" << std::endl;
    return true;
  }

  void createTextures() {
    SDL_Texture *playerTexture =
        g_graphics->createColorTexture(32, 32, 255, 255, 0); // Yellow player
    SDL_Texture *enemyTexture =
        g_graphics->createColorTexture(80, 400, 0, 255, 0); // Green enemy

    if (playerTexture)
      g_graphics->storeTexture("player", playerTexture);
    if (enemyTexture)
      g_graphics->storeTexture("enemy", enemyTexture);
  }

  void createPlayer() {
    auto &playerEntity = entityManager.createEntity();

    playerEntity.addComponent<TransformComponent>(100.0f, 300.0f);
    playerEntity.addComponent<VelocityComponent>(0.0f, 0.0f);
    playerEntity.addComponent<SpriteComponent>(32, 32, 255, 255, 0); // Yellow
    playerEntity.addComponent<ColliderComponent>(32.0f, 32.0f);
    playerEntity.addComponent<InputComponent>();

    player = &playerEntity;
  }

  void handleEvents() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
        running = false;
      }

      if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
        bool isPressed = (event.type == SDL_KEYDOWN);
        if (player && player->hasComponent<InputComponent>()) {
          auto &input = player->getComponent<InputComponent>();
          switch (event.key.keysym.sym) {
          case SDLK_UP:
            input.up = isPressed;
            break;
          case SDLK_DOWN:
            input.down = isPressed;
            break;
          case SDLK_LEFT:
            input.left = isPressed;
            break;
          case SDLK_RIGHT:
            input.right = isPressed;
            break;
          case SDLK_SPACE:
            input.fire = isPressed;
            break;
          default:
            break;
          }
          if (event.key.keysym.sym == SDLK_SPACE && isPressed) {
          }
        }
      }
    }
  }

  void spawnEnemy() {
    float enemyHeight = rng();

    auto &enemy = entityManager.createEntity();
    enemy.addComponent<TransformComponent>(800.0f, enemyHeight);
    enemy.addComponent<VelocityComponent>(ENEMY_SPEED, 0.0f);
    enemy.addComponent<SpriteComponent>(80, static_cast<int>(600 - enemyHeight),
                                        0, 255, 0); // Green
    enemy.addComponent<ColliderComponent>(80.0f, 600 - enemyHeight);
  }

  void update(float deltaTime) {
    if (gameOver)
      return;

    // Update systems
    movementSystem.update(entityManager, deltaTime);
    inputSystem.update(entityManager, deltaTime);
    boundarySystem.update(entityManager, deltaTime);
    cleanupSystem.update(entityManager, deltaTime);
    collisionSystem.update(entityManager);

    // Check game over
    if (player && !player->isActive()) {
      gameOver = true;
    }

    entityManager.refresh();
  }

  void render() {
    g_graphics->clear();

    // Render all entities
    renderSystem.update(entityManager);

    if (gameOver) {
      g_graphics->drawText("Game Over! Press SPACE to restart", 250, 250);
    }

    std::string scoreText = "Score: " + std::to_string(score);
    g_graphics->drawText(scoreText, 10, 10);

    g_graphics->present();
  }

  void restart() {
    entityManager = EntityManager();

    score = 0;
    gameOver = false;

    createPlayer();
  }

  void cleanup() {
    if (g_graphics) {
      delete g_graphics;
      g_graphics = nullptr;
    }
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
};

int main() {
  RTypeGame game;
  game.run();
  return 0;
}