#include "GameLogicSystem.hpp"
#include "enemyFactory.hpp"
#include <vector>

GameLogicSystem::GameLogicSystem() : rng(std::random_device{}())
{
}

void GameLogicSystem::update(EntityManager &entityManager, float deltaTime)
{
    enemySpawnTimer += deltaTime;

    if (currentWave >= waves.size())
        return; // All waves done

    waveTimer += deltaTime;

    // Wait for spawnDelay before starting next wave
    if (!waveActive && waveTimer >= waves[currentWave].spawnDelay)
    {
        std::cout << "Spawning wave " << currentWave + 1 << std::endl;
        spawnWave(entityManager, waves[currentWave]);
        waveActive = true;
    }

    // Check if all enemies from this wave are destroyed
    if (waveActive && entityManager.getEntitiesWithComponents<EnemyComponent>().empty())
    {
        // Prepare for next wave
        waveActive = false;
        waveTimer = 0.0f;
        currentWave++;
    }

    // if (stageStatus == 0)
    // {
    //     stageStatus = 1;
    //     stageCount = 1;
    //     spawnEnemies(entityManager);
    // }
    // else if (stageStatus == 1 && stageCount == 1)
    // {
    //     if (entityManager.getEntitiesWithComponents<LaserWarningComponent>().empty())
    //     {
    //         stageStatus = 1;
    //         stageCount = 2;
    //     }
    // }

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
        enemy.addComponent<HealthComponent>(50, 100);
        enemy.addComponent<HealthBarComponent>(20.0f, 4.0f, -10.0f);
    }
    for (size_t i = 0; i < 1; i++)
    {
        y = 100 + i * 50;

        auto &bonusLife = entityManager.createEntity();
        bonusLife.addComponent<TransformComponent>(700, y);
        bonusLife.addComponent<VelocityComponent>(-230.0f, 0.0f);
        bonusLife.addComponent<SpriteComponent>(20.0f, 20.0f, 0, 255, 0, GraphicsManager::Texture::PLAYER);
        bonusLife.addComponent<ColliderComponent>(20.0f, 20.0f, true);
        std::vector<std::tuple<BonusComponent::TypeBonus, int>> v;
        v.push_back(std::tuple<BonusComponent::TypeBonus, int>(BonusComponent::TypeBonus::HEALTH, 50));
        bonusLife.addComponent<BonusComponent>(v);
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

void GameLogicSystem::spawnWave(EntityManager &entityManager, const Wave &wave)
{
    float cx = windowWidth + 50.0f;
    float cy = windowHeight / 2.0f;
    std::vector<Vector2D> positions = wave.pattern(wave.enemyCount, cx, cy);

    for (const auto &pos : positions)
    {
        EnemyFactory::createEnemy(entityManager, wave.enemyType, pos);
    }
}