
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

void RTypeServer::createPlayer(int playerId)
{
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

    player = &playerEntity;
    _playersScores.push_back({playerId, 0});
}

void RTypeServer::removePlayer(int id)
{
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

    if (tick % 60 == 0)
        std::cout << "-------------60 tick passed--------------" << tick / 60 << std::endl;

    if (gameOver)
        return;

    // 1. Traiter tous les systèmes
    backgroundSystem.update(entityManager, deltaTime);
    playerSystem.update(entityManager, deltaTime);
    inputSystem.update(entityManager, deltaTime);
    if (_state == (GameState)GameState::INGAME)
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
            puts("SENDING SCORE UPDATE");
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
    movementSystem.update(entityManager, deltaTime);
    // 4. Envoyer les updates de mouvement (toutes les entités actives)
    // 2. AVANT applyPendingChanges, envoyer ce qui a été créé/détruit
    sendNewEntities();       // Envoie les entités dans entitiesToCreate
    sendDestroyedEntities(); // Envoie les IDs dans entitiesToDestroy
    sendMovementUpdates();
    entityManager.applyPendingChanges();
    // 4. Envoyer les updates de mouvement (toutes les entités actives)
    sendHealthUpdates();
    sendGameStateUpdates();

    tick++;
}

void RTypeServer::sendGameStateUpdates()
{
    // THINK THATS IT
    mediator.notify(GameMediatorEvent::GameStateUpdate, serializeInt(_state), _lobbyUID);
}

void RTypeServer::sendNewEntities()
{
    // Parcourir les entités créées dans entitiesToCreate
    auto &manager = entityManager;

    // Pour chaque nouvelle entité créée ce tick
    for (const auto &entity : manager.getEntitiesToCreate())
    {
        // std::cout << "Sending new entites" << std::endl;
        // std::cout << "Entity :" << entity->getID() << std::endl;
        auto data = manager.serializeEntityFull(entity->getID());
        std::string serializedData(data.begin(), data.end());
        mediator.notify(GameMediatorEvent::EntityCreated, serializedData, _lobbyUID);
    }
}

void RTypeServer::sendDestroyedEntities()
{
    auto &manager = entityManager;

    // Pour chaque entité détruite ce tick
    for (EntityID id : manager.getEntitiesToDestroy())
    {
        std::cout << "--DESTROY id " << id << std::endl;
        auto data = serializeInt(id);
        mediator.notify(GameMediatorEvent::EntityDestroyed, data, _lobbyUID);
    }
}

void RTypeServer::sendMovementUpdates()
{
    auto data = entityManager.serializeAllMovements();
    std::string serializedData(data.begin(), data.end());
    mediator.notify(GameMediatorEvent::MovementUpdate, serializedData, _lobbyUID);
}

void RTypeServer::sendHealthUpdates()
{
    auto data = entityManager.serializeAllHealth();
    std::string serializedData(data.begin(), data.end());
    int winnerID = -1;
    bool game_over = false;

    for (auto &entity : entityManager.getInactiveEntitiesWithComponents<PlayerComponent>())
    {
        auto &health = entity->getComponent<HealthComponent>();
        auto &playerComp = entity->getComponent<PlayerComponent>();
        std::cout << "Player ID " << playerComp.playerID << " - Health: " << health.health << "/" << health.maxHealth
                  << std::endl;
    }

    std::vector<Entity *> deadPlayers = entityManager.getPlayersDead(winnerID, game_over);

    for (auto *entity : deadPlayers)
    {
        auto &playerComp = entity->getComponent<PlayerComponent>();

        mediator.notify(GameMediatorEvent::PlayerDead, serializeInt(playerComp.playerID), _lobbyUID);
        std::cout << "Player " << playerComp.playerID << " is dead." << std::endl;
    }
    if (game_over && winnerID != -1)
    {
        std::cout << "Player " << winnerID << " is the winner!" << std::endl;
        mediator.notify(GameMediatorEvent::GameOver, serializeInt(winnerID), _lobbyUID);
        this->gameOver = true;
    }
    else if (game_over)
    {
        std::cout << "It's a draw! No winners." << std::endl;
        mediator.notify(GameMediatorEvent::GameOver, serializeInt(-1), _lobbyUID);
        this->gameOver = true;
    }

    mediator.notify(GameMediatorEvent::HealthUpdate, serializedData, _lobbyUID);
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
