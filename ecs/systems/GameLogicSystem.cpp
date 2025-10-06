#include "GameLogicSystem.hpp"

GameLogicSystem::GameLogicSystem() : rng(std::random_device{}())
{
}

void GameLogicSystem::update(EntityManager &entityManager, float deltaTime)
{
    enemySpawnTimer += deltaTime;

    if (stageStatus == 0)
    {
        stageStatus = 1;
        stageCount = 1;
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

int GameLogicSystem::getScore() const
{
    return score;
}

void GameLogicSystem::spawnLaser1(EntityManager &entityManager)
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
        laser.addComponent<TransformComponent>(width / 2, y);
        laser.addComponent<CenteredComponent>(width / 2, height / 2);
        laser.addComponent<SpriteComponent>(width, height, 255, 0, 0);
        laser.addComponent<ColliderComponent>(width, height, false);
        laser.addComponent<LaserWarningComponent>(width, height, waitingTime, 1.5f, 3.0f);
    }
}

void GameLogicSystem::spawnEnemies(EntityManager &entityManager)
{
    float y;
    for (size_t i = 0; i < 10; i++)
    {
        y = 100 + i * 50;

        auto &enemy = entityManager.createEntity();
        enemy.addComponent<TransformComponent>(700, y);
        enemy.addComponent<VelocityComponent>(0.0f, 0.0f);
        enemy.addComponent<SpriteComponent>(20.0f, 20.0f, 0, 255, 0, GraphicsManager::Texture::ENEMY);
        enemy.addComponent<ColliderComponent>(20.0f, 20.0f, true);
        enemy.addComponent<EnemyComponent>(1, 0.2f, 2);
    }
}

void GameLogicSystem::updateScore(EntityManager &entityManager)
{
    auto enemies = entityManager.getEntitiesWithComponents<EnemyComponent, TransformComponent>();
    for (auto &enemy : enemies)
    {
        auto &transform = enemy->getComponent<TransformComponent>();
        auto &enemyComp = enemy->getComponent<EnemyComponent>();
    }
}

void GameLogicSystem::checkGameOverConditions(EntityManager &entityManager)
{
    // Game over logic
}
