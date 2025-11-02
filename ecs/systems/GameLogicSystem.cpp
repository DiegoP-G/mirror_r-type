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
    std::srand(std::time(nullptr));
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
    if (currentWave != 0 && (currentWave + 1) % 5 == 0)
    {
        wave.enemyCount = 1;
        wave.enemyType = "boss";
        wave.pattern = linePattern;
        wave.movement = MOVEMENTTYPE::STATIC_UP_DOWN;
    }
    else
    {
        wave.enemyCount = rand() % 4 + 2;
        std::vector<std::string> enemyTypes = {"basic_enemy", "rotating_enemy", "purple_enemy"};
        wave.enemyType = enemyTypes[rand() % enemyTypes.size()];
        wave.pattern = generateRandomPattern();
        wave.movement = MOVEMENTTYPE::ONLY_LEFT;
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

    if (wave.enemyType == "boss")
    {
        shootingType = SHOOTINGTYPE::CIRCLE;
        positions[0].x -= 140;
    }
    else if (positions.size() <= 3)
    {
        std::vector<SHOOTINGTYPE> possibility{SHOOTINGTYPE::SINUS, SHOOTINGTYPE::THREE_DISPERSED,
                                              SHOOTINGTYPE::ALLDIRECTION};
        shootingType = possibility[rand() % 2];
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
        EnemyFactory::createEnemy(entityManager, wave.enemyType, pos, shootingType, wave.movement);
    }

    spawnBonus(entityManager, cx);
}

void GameLogicSystem::spawnBonus(EntityManager &entityManager, float cx)
{
    auto &bonusLife = entityManager.createEntity();
    float randY = rand() % ((windowHeight - 30) - 10 + 1) + 30;
    float randX = cx;

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

        bonusFiremode.addComponent<AnimatedSpriteComponent>(GraphicsManager::Texture::BONUS_FIREMODE, 96, 0, 32, 32, 5,
                                                            0.1f, 0.0f,
                                                            AnimatedSpriteComponent::SpritesheetLayout::Vertical);
        bonusFiremode.addComponent<ColliderComponent>(20.0f, 20.0f, true);

        std::vector<std::tuple<BonusComponent::TypeBonus, int>> v;
        v.emplace_back(BonusComponent::TypeBonus::FIREMODE, 50);
        bonusFiremode.addComponent<BonusComponent>(v);
    }
    if (rand() % (int)(1 / chance) == 0)
    {
        std::cout << "SPAWING BONUS SHIELD" << std::endl;
        float randY = rand() % ((windowHeight - 30) - 10 + 1) + 30;
        float randX = cx + 50.0f;

        auto &bonusShield = entityManager.createEntity();

        bonusShield.addComponent<TransformComponent>(randX, randY);
        bonusShield.addComponent<VelocityComponent>(-230.0f, 0.0f);

        bonusShield.addComponent<AnimatedSpriteComponent>(GraphicsManager::Texture::BONUS_SHIELD, 32, 0, 32, 32, 5,
                                                          0.1f, 0.0f,
                                                          AnimatedSpriteComponent::SpritesheetLayout::Vertical);
        bonusShield.addComponent<ColliderComponent>(20.0f, 20.0f, true);

        std::vector<std::tuple<BonusComponent::TypeBonus, int>> v;
        v.emplace_back(BonusComponent::TypeBonus::SHIELD, 50);
        bonusShield.addComponent<BonusComponent>(v);
    }
}