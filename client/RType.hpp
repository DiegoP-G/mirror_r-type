#include "../ecs/GraphicsManager.hpp"
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
    // GameLogicSystem gameLogicSystem;

    Entity *player = nullptr;
    bool gameOver = false;

    bool running = false;

    int score = 0;

    const float ENEMY_SPEED = -200.0f;

  public:
    RTypeGame() = default;

    bool init();

    void createTextures();

    void createPlayer();

    void handleEvents();

    void sendInputPlayer();


    void update(float deltaTime);

    void render();

    void restart();

    void run();
};