#include "../ecs/components.hpp"
#include "../ecs/ecs.hpp"
#include "../ecs/systems.hpp"
#include "assetsPath.hpp"
#include <iostream>
#include <random>

class RTypeGame
{
private:
  EntityManager entityManager;

  // Systems
  MovementSystem movementSystem;
  RenderSystem renderSystem;
  CollisionSystem collisionSystem;
  BoundarySystem boundarySystem;        // Generic boundary checking
  OffscreenCleanupSystem cleanupSystem; // Generic cleanup
  InputSystem inputSystem;              // Use existing generic input system
  PlayerSystem playerSystem;
  EnemySystem enemySystem;
  LaserWarningSystem laserWarningSystem;
  GameLogicSystem gameLogicSystem;
  BackgroundSystem backgroundSystem;

  Entity *player = nullptr;
  bool gameOver = false;

  bool running = false;

  int score = 0;

  const float ENEMY_SPEED = -200.0f;

public:
  RTypeGame() = default;

  bool init()
  {
    // Initialize graphics
    g_graphics = new GraphicsManager();
    if (!g_graphics->init("R-Type", 800, 600))
    {
      std::cerr << "Failed to initialize graphics!" << std::endl;
      return false;
    }

    // Create textures
    createTextures();

    // Cretae background
    createBackground();

    // Create player
    createPlayer();

    running = true;

    std::cout << "R-Type initialized!" << std::endl;
    return true;
  }

  void createTextures()
  {
    sf::Texture &backgroundTexture = g_graphics->createTextureFromPath(
      PathFormater::formatAssetPath(backgroundSpritePath), "background");
    sf::Texture &playerTexture = g_graphics->createTextureFromPath(
        PathFormater::formatAssetPath(playerSpritePath),
        "player"); // Yellow player
    sf::Texture &enemyTexture =
        g_graphics->createColorTexture(80, 400, 0, 255, 0); // Green enemy

    g_graphics->storeTexture("background", backgroundTexture);
    g_graphics->storeTexture("player", playerTexture);
    g_graphics->storeTexture("enemy", enemyTexture);
  }

  void createBackground()
  {
    sf::Texture *backgroundTexture = g_graphics->getTexture("background");
    int tileWidth = (int)backgroundTexture->getSize().x;
    int tileHeight = (int)backgroundTexture->getSize().y;

    auto createBackgroundEntity = [&](float x) -> Entity& {
      auto &backgroundEntity = entityManager.createEntity();

      backgroundEntity.addComponent<TransformComponent>
        (x, 0.0f, 1.0f, 1.0f, 0.0f);
      backgroundEntity.addComponent<SpriteComponent>(tileWidth, tileHeight,
        255, 255, 255, backgroundTexture);
      backgroundEntity.addComponent<BackgroundScrollComponent>
        (-300.0f, true);

      return backgroundEntity;
    };

    createBackgroundEntity(0.0f);
    createBackgroundEntity((float)tileWidth);
  }

  void createPlayer()
  {
    auto &playerEntity = entityManager.createEntity();

    playerEntity.addComponent<PlayerComponent>();
    playerEntity.addComponent<TransformComponent>(100.0f, 300.0f);
    playerEntity.addComponent<VelocityComponent>(0.0f, 0.0f);
    playerEntity.addComponent<SpriteComponent>(32, 32, 255, 255, 0); // Yellow
    playerEntity.addComponent<ColliderComponent>(32.0f, 32.0f);
    playerEntity.addComponent<InputComponent>();

    // Load texture and initialize AnimatedSpriteComponent
    sf::Texture *playerTexture = g_graphics->getTexture("player");
    if (playerTexture)
    {
      playerEntity.addComponent<AnimatedSpriteComponent>(
          *playerTexture, 33, 17.5, 0.05f, Vector2D(2.0f, 2.0f));
    }

    player = &playerEntity;
  }

  void handleEvents()
  {
    sf::Event event;
    while (g_graphics->getWindow().pollEvent(event))
    {
      if (event.type == sf::Event::Closed)
      {
        running = false;
      }

      if (event.type == sf::Event::KeyPressed ||
          event.type == sf::Event::KeyReleased)
      {
        if (event.type == sf::Event::KeyPressed ||
            event.type == sf::Event::KeyReleased)
        {
          bool isPressed = (event.type == sf::Event::KeyPressed);
          if (player && player->hasComponent<InputComponent>())
          {
            auto &input = player->getComponent<InputComponent>();
            switch (event.key.code)
            {
            case sf::Keyboard::Up:
              input.up = isPressed;
              break;
            case sf::Keyboard::Down:
              input.down = isPressed;
              break;
            case sf::Keyboard::Left:
              input.left = isPressed;
              break;
            case sf::Keyboard::Right:
              input.right = isPressed;
              break;
            case sf::Keyboard::Space:
              input.fire = isPressed;
              break;
            default:
              break;
            }
          }
        }
      }
    }
  }

  void update(float deltaTime)
  {
    if (gameOver)
      return;

    // Update systems
    gameLogicSystem.update(entityManager, deltaTime);
    backgroundSystem.update(entityManager, deltaTime);
    movementSystem.update(entityManager, deltaTime);
    playerSystem.update(entityManager, deltaTime);
    inputSystem.update(entityManager, deltaTime);
    boundarySystem.update(entityManager, deltaTime);
    cleanupSystem.update(entityManager, deltaTime);
    enemySystem.update(entityManager, deltaTime);
    collisionSystem.update(entityManager);
    laserWarningSystem.update(entityManager, deltaTime);
    // Check game over
    if (player && !player->isActive())
    {
      gameOver = true;
    }

    entityManager.refresh();
  }

  void render()
  {
    g_graphics->clear();

    // Render all entities
    renderSystem.update(entityManager);

    if (gameOver)
    {
      g_graphics->drawText("Game Over! Press SPACE to restart", 250, 250);
    }

    std::string scoreText = "Score: " + std::to_string(score);
    g_graphics->drawText(scoreText, 10, 10);

    g_graphics->present();
  }

  void restart()
  {
    entityManager = EntityManager();

    score = 0;
    gameOver = false;

    createPlayer();
  }

  void cleanup()
  {
    if (g_graphics)
    {
      delete g_graphics;
      g_graphics = nullptr;
    }
  }

  void run()
  {
    if (!init())
    {
      return;
    }

    const float TARGET_FPS = 60.0f;
    const float FRAME_TIME = 1.0f / TARGET_FPS;

    sf::Clock clock;
    float accumulator = 0.0f;

    while (running)
    {
      float deltaTime = clock.restart().asSeconds();

      // Cap delta time to prevent large jumps
      if (deltaTime > 0.05f)
      {
        deltaTime = 0.05f;
      }

      accumulator += deltaTime;

      handleEvents();

      // Fixed timestep update
      while (accumulator >= FRAME_TIME)
      {
        update(FRAME_TIME);
        accumulator -= FRAME_TIME;
      }

      render();
    }

    cleanup();
  }
};

int main()
{
  RTypeGame game;
  game.run();
  return 0;
}