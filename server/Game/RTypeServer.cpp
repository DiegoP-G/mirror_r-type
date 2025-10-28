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

void RTypeServer::createPlayer(int playerId, std::string playerName)
{
    std::lock_guard<std::mutex> lock(entityManager.entityMutex);

    Entity &playerEntity = entityManager.createEntity();

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
    playerEntity.addComponent<TextComponent>(playerName);

    player = &playerEntity;
    _playersScores.push_back({playerId, 0});
}

void RTypeServer::removePlayer(int id)
{
    std::lock_guard<std::mutex> lock(entityManager.entityMutex);

    Entity *entity = getEntityByPlayerID(id);
    if (entity)
    {
        entityManager.markEntityForDestruction(entity->getID());
        std::cout << "[RTypeServer] Player " << id << " removed from the game.\n";
    }
    else
    {
        std::cout << "[RTypeServer] Player " << id << " not found in the game.\n";
    }
}

void RTypeServer::update(float deltaTime)
{
    enum GameState
    {
        MENUIP,
        MENULOBBY,
        LOBBY,
        INGAME,
        MENU,
        GAMEOVER
    };

    // if (_tick % 60 == 0)
    std::cout << "------------- tick passed--------------" << _tick << std::endl;

    if (gameOver)
        return;

    // 1. Traiter tous les systèmes
    {
        std::lock_guard<std::mutex> lock(entityManager.entityMutex);
        backgroundSystem.update(entityManager, deltaTime);
        playerSystem.update(entityManager, deltaTime);
        inputSystem.update(entityManager, deltaTime);
    }

    if ((GameState)_state == (GameState)GameState::INGAME)
    {
        {
            std::lock_guard<std::mutex> lock(entityManager.entityMutex);
            gameLogicSystem.update(entityManager, deltaTime, mediator);

            boundarySystem.update(entityManager, deltaTime);
            cleanupSystem.update(entityManager, deltaTime);
            enemySystem.update(entityManager, deltaTime);
            projectileSystem.update(entityManager, deltaTime);
            bonusSystem.update(entityManager, deltaTime);
        }

        bool updateScore = false;
        {
            std::lock_guard<std::mutex> lock(entityManager.entityMutex);
            collisionSystem.update(entityManager, _playersScores, updateScore);
        }

        if (updateScore)
        {
            puts("SENDING SCORE UPDATE");
            std::lock_guard<std::mutex> lock(entityManager.entityMutex);
            auto serializedScores = entityManager.serializePlayersScores(_playersScores);
            std::string serializedData(serializedScores.begin(), serializedScores.end());
            mediator.notify(GameMediatorEvent::UpdateScore, serializedData, _lobbyUID);
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
            mediator.notify(GameMediatorEvent::GameOver, serializeInt(winnerID), _lobbyUID);
        }
        // laserWarningSystem.update(entityManager, deltaTime);

        // 3. Appliquer les changements (vide les buffers)
    }
    else
    {
        updateLobbyStatus();
    }

    {
        std::lock_guard<std::mutex> lock(entityManager.entityMutex);
        movementSystem.update(entityManager, deltaTime);
    }

    // 4. Envoyer les updates de mouvement (toutes les entités actives)
    // 2. AVANT applyPendingChanges, envoyer ce qui a été créé/détruit
    sendNewEntities(); // Envoie les entités dans entitiesToCreate

    sendDestroyedEntities(); // Envoie les IDs dans entitiesToDestroy

    {
        std::lock_guard<std::mutex> lock(entityManager.entityMutex);
        entityManager.applyPendingChanges();
    }

    // // 4. Envoyer les updates de mouvement (toutes les entités actives)
    sendEntitiesUpdates();
    sendGameStateUpdates();
    std::cout << "Finishing update states" << std::endl;

    _tick++;
}

void RTypeServer::sendGameStateUpdates()
{
    // THINK THATS IT
    mediator.notify(GameMediatorEvent::GameStateUpdate, serializeInt(_state), _lobbyUID);
}

void RTypeServer::sendNewEntities()
{
    auto &manager = entityManager;
    std::vector<uint8_t> data;

    uint32_t tick = _tick;
    data.insert(data.end(), reinterpret_cast<uint8_t *>(&tick), reinterpret_cast<uint8_t *>(&tick) + sizeof(tick));

    uint32_t entityCount = static_cast<uint32_t>(manager.getEntitiesToCreate().size());
    if (entityCount == 0)
        return;
    data.insert(data.end(), reinterpret_cast<uint8_t *>(&entityCount),
                reinterpret_cast<uint8_t *>(&entityCount) + sizeof(entityCount));

    for (const auto &entity : manager.getEntitiesToCreate())
    {
        auto entityData = manager.serializeEntityFull(entity->getID());

        uint32_t entitySize = static_cast<uint32_t>(entityData.size());
        data.insert(data.end(), reinterpret_cast<uint8_t *>(&entitySize),
                    reinterpret_cast<uint8_t *>(&entitySize) + sizeof(entitySize));

        data.insert(data.end(), entityData.begin(), entityData.end());
    }
    std::string serializedData(data.begin(), data.end());
    mediator.notify(GameMediatorEvent::EntitiesCreated, serializedData, _lobbyUID);
}
std::string RTypeServer::serializeAllActiveEntities()
{
    auto &manager = entityManager;
    std::vector<uint8_t> data;

    uint32_t tick = _tick;
    data.insert(data.end(), reinterpret_cast<uint8_t *>(&tick), reinterpret_cast<uint8_t *>(&tick) + sizeof(tick));

    uint32_t entityCount = static_cast<uint32_t>(entityManager.getEntities().size());
    if (entityCount == 0)
        return "";
    data.insert(data.end(), reinterpret_cast<uint8_t *>(&entityCount),
                reinterpret_cast<uint8_t *>(&entityCount) + sizeof(entityCount));

    for (auto &entity : entityManager.getEntities())
    {
        if (entity)
        {
            auto entityData = manager.serializeEntityFull(entity->getID());

            uint32_t entitySize = static_cast<uint32_t>(entityData.size());
            data.insert(data.end(), reinterpret_cast<uint8_t *>(&entitySize),
                        reinterpret_cast<uint8_t *>(&entitySize) + sizeof(entitySize));

            data.insert(data.end(), entityData.begin(), entityData.end());
        }
    }
    std::string serializedData(data.begin(), data.end());
    return serializedData;
}

void RTypeServer::sendDestroyedEntities()
{
    std::lock_guard<std::mutex> lock(entityManager.entityMutex);
    auto &manager = entityManager;

    for (EntityID id : manager.getEntitiesToDestroy())
    {
        std::cout << "--DESTROY id " << id << std::endl;
        auto data = serializeInt(id);
        if (entityManager.getEntityByID(id)->hasComponent<BackgroundScrollComponent>())
        {
            std::cout << "destroy bk " << id << std::endl;
        }
        mediator.notify(GameMediatorEvent::EntityDestroyed, data, _lobbyUID);
    }
}

void RTypeServer::sendEntitiesUpdates()
{
    std::vector<uint8_t> data;

    uint32_t tick = _tick;
    data.insert(data.end(), reinterpret_cast<uint8_t *>(&tick), reinterpret_cast<uint8_t *>(&tick) + sizeof(tick));

    auto moveData = entityManager.serializeAllMovements();
    uint16_t moveSize = static_cast<uint16_t>(moveData.size());
    data.insert(data.end(), reinterpret_cast<uint8_t *>(&moveSize),
                reinterpret_cast<uint8_t *>(&moveSize) + sizeof(moveSize));
    data.insert(data.end(), moveData.begin(), moveData.end());

    auto healthData = entityManager.serializeAllHealth();
    uint16_t healthSize = static_cast<uint16_t>(healthData.size());
    data.insert(data.end(), reinterpret_cast<uint8_t *>(&healthSize),
                reinterpret_cast<uint8_t *>(&healthSize) + sizeof(healthSize));
    data.insert(data.end(), healthData.begin(), healthData.end());

    std::string serializedData(data.begin(), data.end());
    mediator.notify(GameMediatorEvent::UpdateEntities, serializedData, _lobbyUID);
}

void RTypeServer::restart()
{
    std::lock_guard<std::mutex> lock(entityManager.entityMutex);
    entityManager.clear();

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
    // NOTE: This assumes the mutex is already locked by the caller
    // If called independently, wrap the call in a lock_guard
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
    std::lock_guard<std::mutex> lock(entityManager.entityMutex);

    InputComponent inputComp;

    int playerId = deserializePlayerInput(input, inputComp);

    if (playerId != -1)
    {
        auto playerEntity = getEntityByPlayerID(playerId);
        if (playerEntity)
        {
            // ✅ Update existing InputComponent safely
            if (!playerEntity->hasComponent<InputComponent>())
            {
                playerEntity->addComponent<InputComponent>(inputComp);
            }
            else
            {
                auto &existingInput = playerEntity->getComponent<InputComponent>();
                existingInput.fire = inputComp.fire;
                existingInput.up = inputComp.up;
                existingInput.down = inputComp.down;
                existingInput.left = inputComp.left;
                existingInput.right = inputComp.right;
                existingInput.enter = inputComp.enter;
                // copy any other relevant fields
            }

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
    std::lock_guard<std::mutex> lock(entityManager.entityMutex);

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

        std::cout << "Background created" << std::endl;
        return backgroundEntity;
    };

    createBackgroundEntity(0.0f);
    createBackgroundEntity((float)tileWidth);
}

void RTypeServer::sendEntities()
{
    // auto data = entityManager.serializeAllEntities();
    // auto data = entityManager.serializeAllPlayers();
    // std::string serializedData(data.begin(), data.end());
    // mediator.notify(GameMediatorEvent::UpdatePlayers, serializedData);

    // auto dataEnemies = entityManager.serializeAllEnemies();
    // std::string serializedDataEnemies(dataEnemies.begin(), dataEnemies.end());
    // mediator.notify(GameMediatorEvent::UpdateEnemies, serializedDataEnemies);

    // auto dataProjectiles = entityManager.serializeAllProjectiles();
    // std::string serializedDataProjectiles(dataProjectiles.begin(),
    // dataProjectiles.end());
    // mediator.notify(GameMediatorEvent::UpdateProjectiles,
    // serializedDataProjectiles);
}

// Update the number of players and the number of ready players
void RTypeServer::updateLobbyStatus()
{
    std::lock_guard<std::mutex> lock(entityManager.entityMutex);

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

    mediator.notify(GameMediatorEvent::LobbyInfoUpdate, serializedData, _lobbyUID);
    std::cout << "[RTypeServer] Players ready: " << playerReady << " / " << playerNb << std::endl;
}