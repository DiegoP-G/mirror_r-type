
#include "RTypeServer.hpp"
#include "../../ecs/allComponentsInclude.hpp"

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
    if (gameOver)
        return;

    gameLogicSystem.update(entityManager, deltaTime);
    movementSystem.update(entityManager, deltaTime);
    playerSystem.update(entityManager, deltaTime);
    // inputSystem.update(entityManager, deltaTime);
    boundarySystem.update(entityManager, deltaTime);
    cleanupSystem.update(entityManager, deltaTime);
    enemySystem.update(entityManager, deltaTime);
    collisionSystem.update(entityManager);
    laserWarningSystem.update(entityManager, deltaTime);

    if (player && !player->isActive())
    {
        gameOver = true;
    }

    entityManager.refresh();
    sendEntities();
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
    std::cout << "In update" << std::endl;
    update(deltaTime);
    std::cout << "Finish update" << std::endl;
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
    auto data = entityManager.serializeAllEntities();
    std::string serializedData(data.begin(), data.end());
    mediator.notify(GameMediatorEvent::UpdateEntities, serializedData);
}