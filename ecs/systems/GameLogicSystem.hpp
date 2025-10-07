#pragma once

#include "../GraphicsManager.hpp"
#include "../client/windowSize.hpp"
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
#include "../wavePatterns.hpp"
#include <random>

using PatternFunc = std::function<std::vector<Vector2D>(int, float, float)>;

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

    struct Wave
    {
        float spawnDelay; // Time after previous wave
        int enemyCount;
        std::string enemyType;
        PatternFunc pattern;
    };

    // Default waves
    std::vector<Wave> waves{
        {0.0f, 4, "basic_enemy", linePattern},
        {2.5f, 8, "basic_enemy", diamondPattern},
        {2.5f, 7, "basic_enemy", vPattern},
        {2.5f, 10, "basic_enemy", backslashPattern},
    };

    float waveTimer = 0.0f;
    bool waveActive = false;

  public:
    GameLogicSystem();
    void update(EntityManager &entityManager, float deltaTime);
    int getScore() const;
    size_t currentWave = 0;

  private:
    void spawnLaser1(EntityManager &entityManager);
    void spawnEnemies(EntityManager &entityManager);
    void updateScore(EntityManager &entityManager);
    void checkGameOverConditions(EntityManager &entityManager);
    void spawnWave(EntityManager &entityManager, const Wave &wave);
};