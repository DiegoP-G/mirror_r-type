#include "PlayerSystem.hpp"
#include <unordered_map>

static std::unordered_map<size_t, float> warpCooldownTimers;

void PlayerSystem::update(EntityManager &entityManager, float deltaTime, bool client)
{
    auto entities = entityManager.getEntitiesWithComponents<InputComponent, PlayerComponent>();

    for (auto &entity : entities)
    {
        auto &input = entity->getComponent<InputComponent>();
        auto &playerComp = entity->getComponent<PlayerComponent>();
        playerComp.currentCooldown -= deltaTime;

        size_t id = entity->getID();
        if (warpCooldownTimers.find(id) != warpCooldownTimers.end())
        {
            warpCooldownTimers[id] -= deltaTime;
            if (warpCooldownTimers[id] < 0.0f)
                warpCooldownTimers[id] = 0.0f;
        }

        if (input.fire && playerComp.currentCooldown <= 0 && !client)
        {
            fire(entityManager, entity);
            input.fire = false;
            playerComp.currentCooldown = playerComp.attackCooldown;
        }

        if (input.warp)
        {
            handleWarp(entity, deltaTime);
            input.warp = false;
        }

        handlePositionPlayer(entity);
    }
}

void PlayerSystem::handlePositionPlayer(Entity *&entity)
{
    if (!entity->hasComponent<TransformComponent>())
        return;

    auto &transform = entity->getComponent<TransformComponent>();
    if (transform.position.x > 800 - 32)
        transform.position.x = 800 - 32;
    if (transform.position.x < 0)
        transform.position.x = 0;
    if (transform.position.y < 0)
        transform.position.y = 0;
    if (transform.position.y > 600 - 32)
        transform.position.y = 600 - 32;
}

void PlayerSystem::handleWarp(Entity *&entity, float deltaTime)
{
    if (!entity->hasComponent<TransformComponent>() || !entity->hasComponent<InputComponent>())
        return;

    auto &transform = entity->getComponent<TransformComponent>();
    auto &input = entity->getComponent<InputComponent>();
    size_t id = entity->getID();

    if (warpCooldownTimers.find(id) != warpCooldownTimers.end() && warpCooldownTimers[id] > 0.0f)
        return;

    if (input.up)
        transform.position.y -= WARP_DISTANCE;
    else if (input.down)
        transform.position.y += WARP_DISTANCE;
    else if (input.left)
        transform.position.x -= WARP_DISTANCE;
    else if (input.right)
        transform.position.x += WARP_DISTANCE;

    warpCooldownTimers[id] = WARP_COOLDOWN;

    handlePositionPlayer(entity);
}

void PlayerSystem::fire(EntityManager &entityManager, Entity *entity)
{
    auto &transform = entity->getComponent<TransformComponent>();

    auto &bullet = entityManager.createEntity();

    bullet.addComponent<TransformComponent>(transform.position.x + 32.0f, transform.position.y + 16.0f);
    bullet.addComponent<VelocityComponent>(300.0f, 0.0f);
    bullet.addComponent<SpriteComponent>(8, 8, 255, 0, 0);
    bullet.addComponent<ColliderComponent>(8.0f, 8.0f);

    PlayerComponent player = entity->getComponent<PlayerComponent>();

    printf("BULLET OWNER ID: %d\n", player.playerID);
    bullet.addComponent<ProjectileComponent>(30.0f, 2.0f, player.playerID, ENTITY_TYPE::PLAYER);
}
