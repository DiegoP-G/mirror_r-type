#pragma once

#include "../GraphicsManager.hpp"
#include "../components/CenteredComponent.hpp"
#include "../components/ColliderComponent.hpp"
#include "../components/EnemyComponent.hpp"
#include "../components/LaserWarningComponent.hpp"
#include "../components/SpriteComponent.hpp"
#include "../components/TransformComponent.hpp"
#include "../components/VelocityComponent.hpp"
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
    GameLogicSystem() : rng(std::random_device{}())
    {
    }

    void update(EntityManager &entityManager, float deltaTime)
    {
        // Spawn enemies
        enemySpawnTimer += deltaTime;
        // if (enemySpawnTimer >= ENEMY_SPAWN_INTERVAL) {
        //     spawnEnemy(entityManager);
        //     enemySpawnTimer = 0.0f;
        // }
        // std::cout << "stageStatus: " << stageStatus << ", stageCount: " << stageCount << std::endl;
        if (stageStatus == 0)
        {
            stageStatus = 1;
            stageCount = 1;
            // spawnLaser1(entityManager);
            spawnEnemies(entityManager);
        }
        else if (stageStatus == 1 && stageCount == 1)
        {
            if (entityManager.getEntitiesWithComponents<LaserWarningComponent>().empty())
            {
                stageStatus = 1;
                stageCount = 2;
            }
        }

        updateScore(entityManager);

        checkGameOverConditions(entityManager);
    }

    int getScore() const
    {
        return score;
    }

  private:
    void spawnLaser1(EntityManager &entityManager)
    {
        for (size_t i = 0; i < 8; i++)
        {
            float width = 900.0f;
            float height = 50.0f;
            float x = 450.0f;
            float y = 0.0f;
            float waitingTime = 0.0f;
            if (i < 3)
            {
                y = 100.0f + i % 3 * 200;
                waitingTime = 3.0f;
            }
            else if (i < 6)
            {
                y = 100.0f + i % 3 * 200;
                waitingTime = 14.0f;
            }
            else
            {
                y = 200.0f + i % 3 * 200;
                waitingTime = 8.5f;
            }

            auto &laser = entityManager.createEntity();
            laser.addComponent<TransformComponent>(width / 2,
                                                   y); // Centered horizontally
            laser.addComponent<CenteredComponent>(width / 2,
                                                  height / 2); // Centered
            laser.addComponent<SpriteComponent>(width, height, 255, 0,
                                                0); // Full height
            laser.addComponent<ColliderComponent>(width, height, false);
            laser.addComponent<LaserWarningComponent>(width, height, waitingTime, 1.5f,
                                                      3.0f); // Warning for 2s, active for 3s
        }
    }

    void spawnEnemies(EntityManager &entityManager)
    {
        // float enemyHeight = 20.0f;
        float y;
        for (size_t i = 0; i < 10; i++)
        {
            y = 100 + i * 50;

            auto &enemy = entityManager.createEntity();
            enemy.addComponent<TransformComponent>(700, y);
            enemy.addComponent<VelocityComponent>(0.0f, 0.0f);
            enemy.addComponent<SpriteComponent>(20.0f, 20.0f, 0, 255, 0, GraphicsManager::Texture::ENEMY);
            enemy.addComponent<ColliderComponent>(20.0f, 20.0f, true);
            enemy.addComponent<EnemyComponent>(1, 0.2f, 2); // Type 2 = Sine wave movement and 3 bullets
        }
    }

    void updateScore(EntityManager &entityManager)
    {
        // Score logic here - e.g., increment score when enemy passes player
        auto enemies = entityManager.getEntitiesWithComponents<EnemyComponent, TransformComponent>();
        for (auto &enemy : enemies)
        {
            auto &transform = enemy->getComponent<TransformComponent>();
            auto &enemyComp = enemy->getComponent<EnemyComponent>();

            // If enemy passed player position and wasn't counted yet
            // if (transform.position.x < 80.0f && !enemyComp.counted) {
            //     enemyComp.counted = true;
            //     score++;
            // }
        }
    }

    void checkGameOverConditions(EntityManager &entityManager)
    {
        // Additional game over logic here
    }
};
