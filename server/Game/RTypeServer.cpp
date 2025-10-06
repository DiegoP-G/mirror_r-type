
#include "RTypeServer.hpp"
#include "../../ecs/allComponentsInclude.hpp"
#include "../../ecs/GraphicsManager.hpp"
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
    playerEntity.addComponent<PlayerComponent>(playerId, false);
    playerEntity.addComponent<TransformComponent>(100.0f, 300.0f);
    playerEntity.addComponent<VelocityComponent>(0.0f, 0.0f);
    playerEntity.addComponent<SpriteComponent>(32, 32, 255, 255, 0, GraphicsManager::Texture::PLAYER); // Yellow
    playerEntity.addComponent<ColliderComponent>(32.0f, 32.0f);
    playerEntity.addComponent<AnimatedSpriteComponent>(GraphicsManager::Texture::PLAYER, 33, 17.5, 0.05f,
                                                        Vector2D(2.0f, 2.0f));
    playerEntity.addComponent<InputComponent>();

    player = &playerEntity;
}

void RTypeServer::update(float deltaTime)
{
    if (tick % 60 == 0)
        std::cout << "-------------60 tick passed--------------" << tick / 60 << std::endl;

    if (gameOver)
        return;

    // 1. Traiter tous les systèmes
    // gameLogicSystem.update(entityManager, deltaTime);
    backgroundSystem.update(entityManager, deltaTime);

    inputSystem.update(entityManager, deltaTime);
    movementSystem.update(entityManager, deltaTime);
    playerSystem.update(entityManager, deltaTime);
    boundarySystem.update(entityManager, deltaTime);
    cleanupSystem.update(entityManager, deltaTime);
    // enemySystem.update(entityManager, deltaTime);
    // collisionSystem.update(entityManager);
    // laserWarningSystem.update(entityManager, deltaTime);

    // 2. AVANT applyPendingChanges, envoyer ce qui a été créé/détruit
    sendNewEntities(); // Envoie les entités dans entitiesToCreate
                       // sendDestroyedEntities();  // Envoie les IDs dans entitiesToDestroy

    entityManager.applyPendingChanges();
    // 3. Appliquer les changements (vide les buffers)

    // 4. Envoyer les updates de mouvement (toutes les entités actives)
    sendMovementUpdates();

    tick++;
}

void RTypeServer::sendNewEntities()
{
    // Parcourir les entités créées dans entitiesToCreate
    auto &manager = entityManager;

    // Pour chaque nouvelle entité créée ce tick
    for (const auto &entity : manager.getEntitiesToCreate())
    {
        std::cout << "Sending new entites" << std::endl;
        std::cout << "Entity :" << entity->getID() << std::endl;
        auto data = manager.serializeEntityFull(entity->getID());
        std::string serializedData(data.begin(), data.end());
        mediator.notify(GameMediatorEvent::EntityCreated, serializedData);
    }
}

void RTypeServer::sendDestroyedEntities()
{
    auto &manager = entityManager;

    // Pour chaque entité détruite ce tick
    for (EntityID id : manager.getEntitiesToDestroy())
    {
        auto data = serializeInt(id);
        mediator.notify(GameMediatorEvent::EntityDestroyed, data);
    }
}

void RTypeServer::sendMovementUpdates()
{
    auto data = entityManager.serializeAllMovements();
    std::string serializedData(data.begin(), data.end());
    mediator.notify(GameMediatorEvent::MovementUpdate, serializedData);
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

    // Now you can use playerId to find the right player entity
    if (playerId != -1)
    {
        auto playerEntity = getEntityByPlayerID(playerId);
        std::cout << "find player, entity:" << playerEntity->getID() << std::endl;
        if (playerEntity)
        {
            auto currentInputComponent = playerEntity->getComponent<InputComponent>();
            std::cout << "InputComponent values before: ";
            std::cout << "up=" << currentInputComponent.up << ", ";
            std::cout << "down=" << currentInputComponent.down << ", ";
            std::cout << "left=" << currentInputComponent.left << ", ";
            std::cout << "right=" << currentInputComponent.right << ", ";
            std::cout << "shoot=" << currentInputComponent.fire << std::endl;
            std::cout << "InputComponent values before: ";
            playerEntity->addComponent<InputComponent>(inputComp);
            auto currentInputComponentAfter = playerEntity->getComponent<InputComponent>();
            std::cout << "InputComponent values before: ";
            std::cout << "up=" << currentInputComponentAfter.up << ", ";
            std::cout << "down=" << currentInputComponentAfter.down << ", ";
            std::cout << "left=" << currentInputComponentAfter.left << ", ";
            std::cout << "right=" << currentInputComponentAfter.right << ", ";
            std::cout << "shoot=" << currentInputComponentAfter.fire << std::endl;
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
    // std::string serializedDataProjectiles(dataProjectiles.begin(), dataProjectiles.end());
    // mediator.notify(GameMediatorEvent::UpdateProjectiles, serializedDataProjectiles);
}