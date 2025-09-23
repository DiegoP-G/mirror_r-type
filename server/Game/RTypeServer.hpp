#include "../../ecs/components.hpp"
#include "../../ecs/ecs.hpp"
#include "../../ecs/systems.hpp"
#include "assetsPath.hpp"
#include <iostream>
#include <random>

class RTypeServer
{
private:
  EntityManager entityManager;

  MovementSystem movementSystem;
  RenderSystem renderSystem;
  CollisionSystem collisionSystem;
  BoundarySystem boundarySystem;
  OffscreenCleanupSystem cleanupSystem;
  InputSystem inputSystem;
  PlayerSystem playerSystem;
  EnemySystem enemySystem;
  LaserWarningSystem laserWarningSystem;
  GameLogicSystem gameLogicSystem;

  Entity *player = nullptr;
  bool gameOver = false;

  bool running = false;

  int score = 0;

  const float ENEMY_SPEED = -200.0f;

public:
  RTypeServer() = default;

  bool init();

  void createPlayer();

  void update(float deltaTime);

  void restart();

  void cleanup();

  void run();
};