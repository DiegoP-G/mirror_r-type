#pragma once

#include "../../client/windowSize.hpp"
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
#include "../wavePatterns.hpp"
#include "systems/EnemySystem.hpp"
#include <random>

class GameMediator;
using PatternFunc = std::function<std::vector<Vector2D>(int, float, float)>;


constexpr float chance = 1;

struct Wave
{
    int enemyCount;
    std::string enemyType;
    PatternFunc pattern;
};

class GameLogicSystem
{
  private:
    float enemySpawnTimer = 0.0f;
    const float ENEMY_SPEED = -200.0f;
    std::mt19937 rng;
    int score = 0;
    int stageCount = 0;  // 0 = normal, 1 = stage 1, 2 = stage 2, 3 = 1vs 1;
    int stageStatus = 0; // 0 = not started, 1 = in progress, 2 = ended;

    float waveTimer;
    bool waveActive;

  public:
    bool allWavesCompleted = false;
    GameLogicSystem();
    void update(EntityManager &entityManager, float deltaTime, GameMediator &gameMediator, std::string lobbyUid);
    int getScore() const;
    size_t currentWave = 0;

  private:
    void spawnLaser1(EntityManager &entityManager);
    void updateScore(EntityManager &entityManager);
    void checkGameOverConditions(EntityManager &entityManager);
    void spawnWave(EntityManager &entityManager, const Wave &wave);
    Wave generateRandomWave(int currentWave);
    PatternFunc generateRandomPattern();
};