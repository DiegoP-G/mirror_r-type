
#include "RTypeServer.hpp"
#include "../../transferData/transferData.hpp"

bool RTypeServer::init()
{

    running = true;

    std::cout << "R-Type Server initialized!" << std::endl;
    return true;
}

void RTypeServer::createPlayer(const std::string &id)
{
    auto &playerEntity = entityManager.createEntity();

    int playerId = deserializeInt(id);
    playerEntity.addComponent<PlayerComponent>();
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
    inputSystem.update(entityManager, deltaTime);
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

    createPlayer();
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