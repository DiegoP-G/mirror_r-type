
#include "RTypeServer.hpp"
#include "../../ecs/allComponentsInclude.hpp"
#include <string>

bool RTypeServer::init()
{
    running = true;

    return true;
}

void RTypeServer::createPlayer(const std::string &id)
{
    Entity &playerEntity = entityManager.createEntity();

    int playerId = deserializeInt(id);
    playerEntity.addComponent<PlayerComponent>(playerId);
    playerEntity.addComponent<TransformComponent>(100.0f, 300.0f);
    playerEntity.addComponent<VelocityComponent>(0.0f, 0.0f);
    playerEntity.addComponent<SpriteComponent>(32, 32, 255, 255, 0);
    playerEntity.addComponent<ColliderComponent>(32.0f, 32.0f);
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
    gameLogicSystem.update(entityManager, deltaTime);
    movementSystem.update(entityManager, deltaTime);
    playerSystem.update(entityManager, deltaTime);
    boundarySystem.update(entityManager, deltaTime);
    cleanupSystem.update(entityManager, deltaTime);
    enemySystem.update(entityManager, deltaTime);
    collisionSystem.update(entityManager);
    laserWarningSystem.update(entityManager, deltaTime);

    // 2. AVANT applyPendingChanges, envoyer ce qui a été créé/détruit
    sendNewEntities();        // Envoie les entités dans entitiesToCreate
    sendDestroyedEntities();  // Envoie les IDs dans entitiesToDestroy
    
    // 3. Appliquer les changements (vide les buffers)
    entityManager.applyPendingChanges();
    
    // 4. Envoyer les updates de mouvement (toutes les entités actives)
    sendMovementUpdates();

    tick++;
}

void RTypeServer::sendNewEntities()
{
    // Parcourir les entités créées dans entitiesToCreate
    auto& manager = entityManager;
    
    // Pour chaque nouvelle entité créée ce tick
    for (const auto& entity : manager.getEntitiesToCreate())
    {
        auto data = manager.serializeEntityFull(entity->getID());
        std::string serializedData(data.begin(), data.end());
        mediator.notify(GameMediatorEvent::EntityCreated, serializedData);
    }
}

void RTypeServer::sendDestroyedEntities()
{
    auto& manager = entityManager;
    
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
    if (!init())
    {
        return;
    }
    update(deltaTime);
}

// NEED TO ADD THE PLAYER ID TO THE INPUT

void RTypeServer::handlePlayerInput(const std::string &input)
{
    InputComponent inputComp;

    int playerId = deserializePlayerInput(input, inputComp);

    // Now you can use playerId to find the right player entity
    std::cout << input << std::endl;
    std::cout << playerId << std::endl;
    if (playerId != -1)
    {
        auto playerEntity = entityManager.getEntityByID(playerId);
        if (playerEntity)
        {
            playerEntity->addComponent<InputComponent>(inputComp);
        }
    }
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