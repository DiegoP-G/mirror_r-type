
#include "RTypeServer.hpp"
#include "../../ecs/allComponentsInclude.hpp"

bool RTypeServer::init()
{
    running = true;

    std::cout << "R-Type Server initialized!" << std::endl;
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
    std::cout << "Server ecs running..." << std::endl;
    if (!init())
    {
        return;
    }

    update(deltaTime);
}

// NEED TO ADD THE PLAYER ID TO THE INPUT

void RTypeServer::handlePlayerInput(const std::string &input)
{
    auto entities = entityManager.getEntitiesWithComponent<InputComponent>();

    auto newInputComponent = InputComponent();
    const uint8_t *inputData = reinterpret_cast<const uint8_t *>(input.data());

    newInputComponent.deserialize(inputData);

    for (auto &entity : entities)
    {

        // Now it neeed to get the right player with player id and change the input.
    }
}

void RTypeServer::sendEntities()
{
    auto data = entityManager.serializeAllEntities();
    std::string serializedData(data.begin(), data.end());
    mediator.notify(GameMediatorEvent::UpdateEntities, serializedData);
}