#pragma once

#include "../GraphicsManager.hpp"
#include "../components/BonusComponent.hpp"
#include "../components/CenteredComponent.hpp"
#include "../components/ColliderComponent.hpp"
#include "../components/EnemyComponent.hpp"
#include "../components/LaserWarningComponent.hpp"
#include "../components/SpriteComponent.hpp"
#include "../components/TransformComponent.hpp"
#include "../components/VelocityComponent.hpp"

#include "../components/HealthBarComponent.hpp"
#include "../components/HealthComponent.hpp"
#include "../entityManager.hpp"
#include <random>

// System for game rules and logic
class GameLogicSystem
{
  private:
    float enemySpawnTimer = 0.0f;
    const float ENEMY_SPAWN_INTERVAL = 2.0f;
    const float ENEMY_SPEED = -200.0f;
    std::mt19937 rng;
    int score = 0;
    int stageCount = 0;  // 0 = normal, 1 = stage 1, 2 = stage 2, 3 = 1vs 1;
    int stageStatus = 0; // 0 = not started, 1 = in progress, 2 = ended;
  public:
    GameLogicSystem();
    void update(EntityManager &entityManager, float deltaTime);
    int getScore() const;

  private:
    void spawnLaser1(EntityManager &entityManager);
    void spawnEnemies(EntityManager &entityManager);
    void updateScore(EntityManager &entityManager);
    void checkGameOverConditions(EntityManager &entityManager);
};