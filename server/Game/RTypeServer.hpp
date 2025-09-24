#pragma once
#include "../../ecs/components.hpp"
#include "../../ecs/ecs.hpp"
#include "../../ecs/systems.hpp"
#include "../../transferData/transferData.hpp"
#include "GameMediator.hpp"

#include <iostream>
#include <random>

class GameMediator;


class RTypeServer
{
private:
  GameMediator &mediator;
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
  RTypeServer(GameMediator &mediator) : mediator(mediator) {}

  bool init();

  void createPlayer(const std::string &);

  void update(float deltaTime);

  void restart();

  void cleanup();

  void run(float deltaTime);

  void sendEntities();

  void handlePlayerInput(const std::string&);
};