#include "../../ecs/components.hpp"
#include "../../ecs/ecs.hpp"
#include "../../ecs/systems.hpp"
#include "assetsPath.hpp"
#include <iostream>
#include <random>
#include "RTypeServer.hpp"


  bool RTypeServer::init()
  {


    createPlayer();

    running = true;

    std::cout << "R-Type Server initialized!" << std::endl;
    return true;
  }

  void RTypeServer::createPlayer()
  {
    auto &playerEntity = entityManager.createEntity();

    playerEntity.addComponent<PlayerComponent>();
    playerEntity.addComponent<TransformComponent>(100.0f, 300.0f);
    playerEntity.addComponent<VelocityComponent>(0.0f, 0.0f);
    // playerEntity.addComponent<SpriteComponent>(32, 32, 255, 255, 0);
    playerEntity.addComponent<ColliderComponent>(32.0f, 32.0f);
    playerEntity.addComponent<InputComponent>();

    player = &playerEntity;
  }

  void RTypeServer::update(float deltaTime)
  {
    if (gameOver)
      return;

    gameLogicSystem.update(entityManager, deltaTime);
    movementSystem.update(entityManager, deltaTime);
    playerSystem.update(entityManager, deltaTime);
    inputSystem.update(entityManager, deltaTime);
    boundarySystem.update(entityManager, deltaTime);
    cleanupSystem.update(entityManager, deltaTime);
    enemySystem.update(entityManager, deltaTime);
    collisionSystem.update(entityManager);
    laserWarningSystem.update(entityManager, deltaTime);
    if (player && !player->isActive())
    {
      gameOver = true;
    }

    entityManager.refresh();
  }


  void RTypeServer::restart()
  {
    entityManager = EntityManager();

    score = 0;
    gameOver = false;

    createPlayer();
  }

  void RTypeServer::cleanup()
  {
    return;
  };

  void RTypeServer::run()
  {
    if (!init())
    {
      return;
    }

    const float TARGET_FPS = 60.0f;
    const float FRAME_TIME = 1.0f / TARGET_FPS;

    sf::Clock clock;
    float accumulator = 0.0f;

    float deltaTime = clock.restart().asSeconds();

    if (deltaTime > 0.05f)
    {
      deltaTime = 0.05f;
    }

    accumulator += deltaTime;
    while (accumulator >= FRAME_TIME)
    {
      update(FRAME_TIME);
      accumulator -= FRAME_TIME;
    }
  }