
#include "RTypeServer.hpp"
#include "../../ecs/GraphicsManager.hpp"
#include "../../ecs/allComponentsInclude.hpp"
#include <cstddef>
#include <string>

bool RTypeServer::init()
{
    running = true;
    createBackground();

    return true;
}

void RTypeServer::createPlayer(const std::string &id)
{
    Entity &playerEntity = entityManager.createEntity();

    int playerId = deserializeInt(id);
    playerEntity.addComponent<PlayerComponent>(playerId, false, 0.25f);
    playerEntity.addComponent<TransformComponent>(100.0f, 300.0f);
    playerEntity.addComponent<VelocityComponent>(0.0f, 0.0f);
    playerEntity.addComponent<SpriteComponent>(32, 32, 255, 255, 0, GraphicsManager::Texture::PLAYER); // Yellow
    playerEntity.addComponent<ColliderComponent>(32.0f, 32.0f);
    playerEntity.addComponent<AnimatedSpriteComponent>(GraphicsManager::Texture::PLAYER, 0, 0, 33, 17.5, 5, 0.05f, 0.0f,
                                                       Vector2D(2.0f, 2.0f));
    playerEntity.addComponent<InputComponent>();
    playerEntity.addComponent<HealthComponent>(100, 150);
    playerEntity.addComponent<HealthBarComponent>(50.0f, 4.0f, -10.0f);

    player = &playerEntity;
    _playersScores.push_back({playerId, 0});
}

void RTypeServer::update(float deltaTime)
{

    std::cout << "-------------60 tick passed--------------" << _tick << std::endl;

    if (gameOver)
        return;

    backgroundSystem.update(entityManager, deltaTime);
    playerSystem.update(entityManager, deltaTime);
    inputSystem.update(entityManager, deltaTime);
    if (_state == GameState::INGAME)
    {
        gameLogicSystem.update(entityManager, deltaTime, mediator);

        boundarySystem.update(entityManager, deltaTime);
        cleanupSystem.update(entityManager, deltaTime);
        enemySystem.update(entityManager, deltaTime);
        projectileSystem.update(entityManager, deltaTime);
        bonusSystem.update(entityManager, deltaTime);

        bool updateScore = false;
        collisionSystem.update(entityManager, _playersScores, updateScore);
        if (updateScore)
        {
            auto serializedScores = entityManager.serializePlayersScores(_playersScores);
            std::string serializedData(serializedScores.begin(), serializedScores.end());
            mediator.notify(GameMediatorEvent::UpdateScore, serializedData);
        }
        if (gameLogicSystem.allWavesCompleted && !gameOver)
        {
            std::cout << "[Server] 🏆 All waves completed! Determining winner..." << std::endl;

            // Trouver le gagnant par score
            int winnerID = -1;
            int maxScore = -1;

            for (auto &[playerId, score] : _playersScores)
            {
                std::cout << "[Server] Player " << playerId << " score: " << score << std::endl;
                if (score > maxScore)
                {
                    maxScore = score;
                    winnerID = playerId;
                }
            }

            gameOver = true;
            std::cout << "[Server] Winner: Player " << winnerID << " with score " << maxScore << std::endl;
            mediator.notify(GameMediatorEvent::GameOver, serializeInt(winnerID));
        }
        // laserWarningSystem.update(entityManager, deltaTime);

        // 3. Appliquer les changements (vide les buffers)
    }
    else
    {
        updateLobbyStatus();
    }
    movementSystem.update(entityManager, deltaTime);

    sendNewEntities();
    sendDestroyedEntities();

    entityManager.applyPendingChanges();
    sendEntitiesUpdates();
    sendGameStateUpdates();

    _tick++;
}

void RTypeServer::restart()
{
    entityManager = EntityManager();

    score = 0;
    gameOver = false;
}

void RTypeServer::cleanup()
{
    return;
};

void RTypeServer::run(float deltaTime)
{

    if (player != nullptr)
        update(deltaTime);
}

Entity *RTypeServer::getEntityByPlayerID(int playerID)
{
    auto players = entityManager.getEntitiesWithComponent<PlayerComponent>();
    for (auto *entity : players)
    {
        auto &playerComp = entity->getComponent<PlayerComponent>();
        if (playerComp.playerID == playerID)
            return entity;
    }
    return nullptr;
}

// NEED TO ADD THE PLAYER ID TO THE INPUT
void RTypeServer::handlePlayerInput(const std::string &input)
{
    InputComponent inputComp;

    int playerId = deserializePlayerInput(input, inputComp);

    if (playerId != -1)
    {
        auto playerEntity = getEntityByPlayerID(playerId);
        if (playerEntity)
        {
            playerEntity->addComponent<InputComponent>(inputComp);

            if (inputComp.enter)
            {
                auto &playerComp = playerEntity->getComponent<PlayerComponent>();
                playerComp.isReady = true; // mark player as ready
            }
        }
    }
}

void RTypeServer::createBackground()
{
    // sf::Texture *backgroundTexture = g_graphics->getTexture("background");
    std::cout << "Background created" << std::endl;
    int tileWidth = 800;
    int tileHeight = 600;

    auto createBackgroundEntity = [&](float x) -> Entity & {
        auto &backgroundEntity = entityManager.createEntity();

        backgroundEntity.addComponent<TransformComponent>(x, 0.0f, 1.0f, 1.0f, 0.0f);
        backgroundEntity.addComponent<SpriteComponent>(tileWidth, tileHeight, 255, 255, 255,
                                                       GraphicsManager::Texture::BACKGROUND);
        backgroundEntity.addComponent<BackgroundScrollComponent>(-300.0f, true);

        return backgroundEntity;
    };

    createBackgroundEntity(0.0f);
    createBackgroundEntity((float)tileWidth);
}

// Update the number of players and the number of ready players
void RTypeServer::updateLobbyStatus()
{
    auto players = entityManager.getEntitiesWithComponent<PlayerComponent>();
    playerNb = static_cast<int>(players.size());
    playerReady = 0;

    for (auto *entity : players)
    {
        auto &playerComp = entity->getComponent<PlayerComponent>();
        if (playerComp.isReady)
        {
            playerReady++;
        }
    }

    if (playerNb >= 1 && (playerReady == playerNb))
    {
        _state = GameState::INGAME;
    }
    std::vector<uint8_t> payload = {(uint8_t)playerReady, (uint8_t)playerNb};
    std::string serializedData(payload.begin(), payload.end());

    mediator.notify(GameMediatorEvent::LobbyInfoUpdate, serializedData);
    std::cout << "[RTypeServer] Players ready: " << playerReady << " / " << playerNb << std::endl;
}
