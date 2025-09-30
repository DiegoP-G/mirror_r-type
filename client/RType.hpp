#include "../ecs/components.hpp"
#include "../ecs/systems.hpp"
#include "NetworkECSMediator.hpp"
#include <mutex>

class RTypeGame
{
  private:
    std::mutex _mutex;
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

    NetworkECSMediator _med;

    Entity *player = nullptr;
    bool gameOver = false;

    bool running = false;

    int score = 0;

    const float ENEMY_SPEED = -200.0f;

  public:
    RTypeGame() = default;

    std::mutex& getMutex()
    {
      return _mutex;
    }

    EntityManager &getEntityManager()
    {
        return entityManager;
    }

    bool init(NetworkECSMediator med);

    void createTextures();

    void createBackground();

    void createPlayer();

    void handleEvents();

    void sendInputPlayer();

    void update(float deltaTime);

    void render();

    void restart();

    void run();
};