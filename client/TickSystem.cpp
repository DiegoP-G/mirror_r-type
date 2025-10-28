#include "TickSystem.hpp"
#include "../ecs/components/PlayerComponent.hpp"
#include "../ecs/ecs.hpp"
#include "../ecs/systems.hpp"
#include <algorithm>
#include <chrono>

void TickSystem::update(EntityManager &entityManager)
{
    if (!predictionEnabled)
        return;

    static auto previousTime = std::chrono::high_resolution_clock::now();
    auto currentTime = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double> frameTime = currentTime - previousTime;
    previousTime = currentTime;

    tickAccumulator += frameTime.count();

    while (tickAccumulator >= TICK_DURATION)
    {
        processTick(entityManager);
        tickAccumulator -= TICK_DURATION;
    }
}

void TickSystem::processTick(EntityManager &entityManager)
{
    currentTick++;

    PlayerInput input = capturePlayerInput(entityManager);
    input.tickNumber = currentTick;
    input.sequenceNumber = lastProcessedInputSequence++;
    inputHistory.push_back(input);

    applyPlayerInputs(entityManager, input);
    simulatePlayerOnly(entityManager);

    saveSnapshot(entityManager);

    cleanupHistory();
}

void TickSystem::applyPlayerInputs(EntityManager &entityManager, const PlayerInput &input)
{
    auto entities = entityManager.getEntitiesWithComponents<PlayerComponent, InputComponent>();

    for (auto &entity : entities)
    {
        if (entity->getComponent<PlayerComponent>().playerID != _playerId)
            continue;

        auto &velocity = entity->getComponent<VelocityComponent>();
        auto &transform = entity->getComponent<TransformComponent>();
        auto &playerComp = entity->getComponent<PlayerComponent>();
        auto &inputComp = entity->getComponent<InputComponent>();

        // ! apply movement input
        velocity.velocity.x = 0.0f;
        velocity.velocity.y = 0.0f;

        const float PLAYER_SPEED = 200.0f;
        if (input.moveUp)
            velocity.velocity.y = -PLAYER_SPEED;
        if (input.moveDown)
            velocity.velocity.y = PLAYER_SPEED;
        if (input.moveLeft)
            velocity.velocity.x = -PLAYER_SPEED;
        if (input.moveRight)
            velocity.velocity.x = PLAYER_SPEED;

        transform.position += velocity.velocity * TICK_DURATION;

        // if (inputComp.warp)
        // {
        //     if (playerComp.stamina >= WARP_STAMINA_COST && playerComp.warpCooldown <= 0.0f)
        //     {
        //         if (inputComp.up)
        //             transform.position.y -= WARP_DISTANCE;
        //         else if (inputComp.down)
        //             transform.position.y += WARP_DISTANCE;
        //         else if (inputComp.left)
        //             transform.position.x -= WARP_DISTANCE;
        //         else if (inputComp.right)
        //             transform.position.x += WARP_DISTANCE;
        //     }
        // }
    }
}

void TickSystem::simulatePlayerOnly(EntityManager &entityManager)
{
    playerSystem.update(entityManager, TICK_DURATION, true);
}

PlayerInput TickSystem::capturePlayerInput(EntityManager &clientEm)
{
    PlayerInput input{};
    input.moveUp = false;
    input.moveDown = false;
    input.moveLeft = false;
    input.moveRight = false;
    input.shoot = false;

    Entity *player = nullptr;
    for (auto &entity : clientEm.getEntities())
    {
        if (entity->hasComponent<PlayerComponent>())
            std::cout << "Player found with id : " << entity->getComponent<PlayerComponent>().playerID << " my id is "
                      << _playerId << std::endl;
        if (entity->hasComponent<PlayerComponent>() && entity->getComponent<PlayerComponent>().playerID == _playerId)
        {
            player = entity.get();
            break;
        }
    }

    if (!player)
    {
        std::cout << "❌ Player not found for capture input!" << std::endl;
        return input;
    }

    auto &inputC = player->getComponent<InputComponent>();

    input.moveUp = inputC.up;
    input.moveDown = inputC.down;
    input.moveLeft = inputC.left;
    input.moveRight = inputC.right;
    input.shoot = inputC.fire;

    return input;
}

void TickSystem::saveSnapshot(EntityManager &entityManager)
{
    GameSnapshot snapshot;
    snapshot.tickNumber = currentTick;
    snapshot.timestamp = currentTick * TICK_DURATION;
    snapshot.valid = true;

    for (auto &entity : entityManager.getEntities())
    {
        if (entity->hasComponent<PlayerComponent>() && entity->getComponent<PlayerComponent>().playerID == _playerId)
        {
            PlayerSnapshot playerSnap;
            playerSnap.entityId = entity->getID();

            auto &transform = entity->getComponent<TransformComponent>();
            playerSnap.x = transform.position.x;
            playerSnap.y = transform.position.y;

            auto &vel = entity->getComponent<VelocityComponent>();
            playerSnap.vx = vel.velocity.x;
            playerSnap.vy = vel.velocity.y;

            auto &health = entity->getComponent<HealthComponent>();
            playerSnap.health = health.health;

            auto &player = entity->getComponent<PlayerComponent>();
            playerSnap.score = player.score;

            snapshot.players.push_back(playerSnap);
        }
    }

    std::cout << "new snap " << currentTick << std::endl;
    snapshotHistory.push_back(snapshot);
}

bool TickSystem::statesDiffer(GameSnapshot &clientSnapshot, EntityManager &serverEM)
{
    const float POSITION_TOLERANCE = 1.0f;

    if (clientSnapshot.players.empty())
        return true;

    Entity *serverPlayer = nullptr;
    for (auto &entity : serverEM.getEntitiesWithComponents<PlayerComponent>())
    {
        if (entity->getComponent<PlayerComponent>().playerID == _playerId)
        {
            serverPlayer = entity;
            break;
        }
    }

    if (!serverPlayer)
    {
        std::cout << "PLAYER NOT FOUND FOR ROLLBACK" << std::endl;
        return true;
    }

    const PlayerSnapshot &clientPlayer = clientSnapshot.players[0];

    auto &serverTransform = serverPlayer->getComponent<TransformComponent>();

    float dx = std::abs(clientPlayer.x - serverTransform.position.x);
    float dy = std::abs(clientPlayer.y - serverTransform.position.y);

    if (dx > POSITION_TOLERANCE || dy > POSITION_TOLERANCE)
    {
        std::cout << "❌  ROLLBACK NEEDED" << std::endl;
        std::cout << "Position differs: dx=" << dx << " dy=" << dy << std::endl;
        return true;
    }

    std::cout << "✅ NO ROLLBACK NEEDED" << std::endl;
    return false;
}

void TickSystem::performRollback(uint32_t serverTick, EntityManager &serverEM, EntityManager &clientEM)
{
    std::cout << "🔄 ROLLBACK from tick " << currentTick << " to " << serverTick << std::endl;

    Entity *player = nullptr;
    for (auto *entity : clientEM.getEntitiesWithComponents<PlayerComponent>())
    {
        if (entity->getComponent<PlayerComponent>().playerID == _playerId)
        {
            player = entity;
            break;
        }
    }

    if (!player)
    {
        std::cout << "❌ Player not found for rollback!\n";
        return;
    }

    Entity *serverPlayer = nullptr;
    for (auto *entity : serverEM.getEntitiesWithComponents<PlayerComponent>())
    {
        if (entity->getComponent<PlayerComponent>().playerID == _playerId)
        {
            serverPlayer = entity;
            break;
        }
    }

    if (!serverPlayer)
    {
        std::cout << "❌ Server player not found!\n";
        return;
    }

    auto &serverTransform = serverPlayer->getComponent<TransformComponent>();

    auto &clientTransform = player->getComponent<TransformComponent>();
    clientTransform = serverTransform;

    auto &serverVel = serverPlayer->getComponent<VelocityComponent>();

    auto &clientVel = player->getComponent<VelocityComponent>();
    clientVel = serverVel;

    for (const auto &input : inputHistory)
    {
        if (input.tickNumber > serverTick && input.tickNumber <= currentTick)
        {
            applyPlayerInputs(clientEM, input);
            simulatePlayerOnly(clientEM);
        }
    }

    std::cout << "✅ Rollback complete, resimulated " << (currentTick - serverTick) << " ticks" << std::endl;
}

void TickSystem::onServerUpdate(uint32_t serverTick, EntityManager &serverEM, EntityManager &clientEm)
{
    lastConfirmedServerTick = serverTick;

    auto snapshot = getSnapshot(serverTick);

    if (!snapshot.valid)
    {
        std::cout << "⚠️ Snapshot too old, force sync" << std::endl;
        forceStateUpdate(clientEm, serverEM);
        return;
    }

    if (statesDiffer(snapshot, serverEM))
    {
        performRollback(serverTick, serverEM, clientEm);
    }
}

void TickSystem::forceStateUpdate(EntityManager &clientEm, EntityManager &serverEM)
{
    saveSnapshot(clientEm);
    snapshotHistory.clear();
    inputHistory.clear();
}

void TickSystem::cleanupHistory()
{
    while (snapshotHistory.size() > MAX_SNAPSHOTS)
    {
        snapshotHistory.pop_front();
    }

    while (!inputHistory.empty() && inputHistory.front().tickNumber < lastConfirmedServerTick)
    {
        inputHistory.pop_front();
    }
}

GameSnapshot TickSystem::getSnapshot(uint32_t tick)
{
    for (auto it = snapshotHistory.rbegin(); it != snapshotHistory.rend(); ++it)
    {
        if (it->tickNumber == tick)
        {
            return *it;
        }
    }

    GameSnapshot invalidSnap;
    invalidSnap.valid = false;
    return invalidSnap;
}
