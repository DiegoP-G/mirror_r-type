#include "GameLogicSystem.hpp"
#include "../enemyFactory.hpp"
#include "../server/Game/GameMediator.hpp"
#include "../server/Lobby/Lobby.hpp"
#include "../server/Lobby/LobbyManager.hpp"
#include "../transferData/transferData.hpp"
#include <cstdlib>
#include <ctime>
#include <vector>

GameLogicSystem::GameLogicSystem() : rng(std::random_device{}())
{
    waveActive = false;
    waveTimer = 0.0f;
}

void GameLogicSystem::update(EntityManager &entityManager, float deltaTime, GameMediator &gameMediator,
                             std::string lobbyUid)
{
    enemySpawnTimer += deltaTime;
    waveTimer += deltaTime;

    // Wait for spawnDelay before starting next wave
    if (!waveActive && waveTimer >= 1)
    {
        spawnWave(entityManager, generateRandomWave(currentWave));
        waveActive = true;

        auto players = entityManager.getEntitiesWithComponent<PlayerComponent>();
        for (auto *player : players)
        {
            auto &playerComp = player->getComponent<PlayerComponent>();
            gameMediator.notify(GameMediatorEvent::NewWave, "", "", playerComp.playerID);
        }
    }
    else if (waveActive && entityManager.getEntitiesWithComponents<EnemyComponent>().empty())
    {
        // Prepare for next wave
        waveActive = false;
        waveTimer = 0.0f;
        currentWave++;

        // Notify the server about the wave change
        std::string waveData = serializeInt(currentWave);
        gameMediator.notify(GameMediatorEvent::UpdateWave, waveData, lobbyUid);
    }

    auto players = entityManager.getEntitiesWithComponent<PlayerComponent>();
    for (auto *player : players)
    {
        auto &playerComp = player->getComponent<PlayerComponent>();
        if (playerComp.bonusPicked)
        {
            gameMediator.notify(GameMediatorEvent::PlayerBonus, "", "", playerComp.playerID);
            playerComp.bonusPicked = false;
        }
    }

    updateScore(entityManager);
    checkGameOverConditions(entityManager);
}

Wave GameLogicSystem::generateRandomWave(int currentWave)
{
    Wave wave;
    // Boss wave
    if (currentWave != 0 && currentWave % 5 == 0)
    {
        wave.enemyCount = 1;
        wave.enemyType = "boss";
        wave.pattern = linePattern;
    }
    else
    {
        wave.enemyCount = rand() % 4 + 3;
        wave.enemyType = (rand() % 2 == 0) ? "basic_enemy" : "rotating_enemy";
        wave.pattern = generateRandomPattern();
    }
    return wave;
}

PatternFunc GameLogicSystem::generateRandomPattern()
{
    std::vector<PatternFunc> patterns = {linePattern, diamondPattern, vPattern, backslashPattern, randomPattern};
    return patterns[rand() % patterns.size()];
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
    SHOOTINGTYPE shootingType;

    if (positions.size() <= 3)
    {
        shootingType = (rand() % 2 == 0) ? SHOOTINGTYPE::SINUS : SHOOTINGTYPE::THREE_DISPERSED;
    }
    else if (positions.size() <= 5)
    {
        // Not so many enemy, shootingType = sinus/3 bullets
        shootingType = (rand() % 2 == 0) ? SHOOTINGTYPE::STRAIGHT : SHOOTINGTYPE::SINUS;
    }
    else
    {
        shootingType = SHOOTINGTYPE::STRAIGHT;
    }

    for (const auto &pos : positions)
    {
        EnemyFactory::createEnemy(entityManager, wave.enemyType, pos, shootingType);
    }

    // === Spawn a bonus life entity ===
    auto &bonusLife = entityManager.createEntity();

    // Random position inside the window (you had a syntax error here)
    // float randX = static_cast<float>(rand() % static_cast<int>(windowWidth));

    float randY = rand() % ((windowHeight - 30) - 10 + 1) + 30;

    float randX = cx;
    std::cout << "SPWAN BONUS on " << randX << "  " << randY << std::endl;

    bonusLife.addComponent<TransformComponent>(randX, randY);
    bonusLife.addComponent<VelocityComponent>(-230.0f, 0.0f);

    bonusLife.addComponent<AnimatedSpriteComponent>(GraphicsManager::Texture::BONUS_LIFE, 0.0, 0.0, 32, 32, 5, 0.1f,
                                                    0.0f, AnimatedSpriteComponent::SpritesheetLayout::Vertical);
    bonusLife.addComponent<ColliderComponent>(20.0f, 20.0f, true);

    std::vector<std::tuple<BonusComponent::TypeBonus, int>> v;
    v.emplace_back(BonusComponent::TypeBonus::HEALTH, 50);
    bonusLife.addComponent<BonusComponent>(v);

    if (rand() % (int)(1 / chance) == 0)
    {
        float randY = rand() % ((windowHeight - 30) - 10 + 1) + 30;
        float randX = cx + 50.0f;

        auto &bonusFiremode = entityManager.createEntity();

        bonusFiremode.addComponent<TransformComponent>(randX, randY);
        bonusFiremode.addComponent<VelocityComponent>(-230.0f, 0.0f);

        bonusFiremode.addComponent<AnimatedSpriteComponent>(GraphicsManager::Texture::BONUS_FIREMODE, 93, 0, 32, 32, 5,
                                                            0.1f, 0.0f,
                                                            AnimatedSpriteComponent::SpritesheetLayout::Vertical);
        bonusFiremode.addComponent<ColliderComponent>(20.0f, 20.0f, true);

        std::vector<std::tuple<BonusComponent::TypeBonus, int>> v;
        v.emplace_back(BonusComponent::TypeBonus::FIREMODE, 50);
        bonusFiremode.addComponent<BonusComponent>(v);
    }
}