#include "../ecs/components.hpp"
#include "../ecs/systems.hpp"
#include "NetworkECSMediator.hpp"
#include "../ecs/waveSystem.hpp"

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
    WaveSystem waveSystem;
    LaserWarningSystem laserWarningSystem;
    GameLogicSystem gameLogicSystem;

    NetworkECSMediator _med;

    Entity *player = nullptr;
    bool gameOver = false;

    bool running = false;

    int score = 0;

    const float ENEMY_SPEED = -200.0f;

  public:
    RTypeGame() = default;

    bool init(NetworkECSMediator med);

    void createTextures();

    void createPlayer();

    void handleEvents();

    void sendInputPlayer();

    void update(float deltaTime);

    void render();

    void restart();

    void run();
};