#include "PlayerSystem.hpp"

void PlayerSystem::update(EntityManager &entityManager, float deltaTime, bool client)
{
    auto entities = entityManager.getEntitiesWithComponents<InputComponent, PlayerComponent>();

    for (auto &entity : entities)
    {
        auto &input = entity->getComponent<InputComponent>();
        auto &playerComp = entity->getComponent<PlayerComponent>();
        playerComp.currentCooldown -= deltaTime;

        // ! initialize stamina if missing (safety net)
        if (playerComp.stamina > playerComp.maxStamina)
            playerComp.stamina = playerComp.maxStamina;

        // ! stamina regeneration
        playerComp.stamina += playerComp.staminaRegenRate * deltaTime;
        if (playerComp.stamina > playerComp.maxStamina)
            playerComp.stamina = playerComp.maxStamina;

        // ! warp cooldown update
        if (playerComp.warpCooldown > 0.0f)
        {
            playerComp.warpCooldown -= deltaTime;
            if (playerComp.warpCooldown < 0.0f)
                playerComp.warpCooldown = 0.0f;
        }

        std::cout << "STAMINA" << playerComp.stamina << std::endl;
        if (input.fire && playerComp.currentCooldown <= 0 && !client)
        {
            if (playerComp.stamina >= FIRE_STAMINA_COST)
            {
                fire(entityManager, entity);
            }
        }
        // ! Had to add this to bypass !client causing issues
        if (input.fire && playerComp.currentCooldown <= 0)
        {
            if (playerComp.stamina >= FIRE_STAMINA_COST)
            {
                playerComp.stamina -= FIRE_STAMINA_COST;
                input.fire = false;
                playerComp.currentCooldown = playerComp.attackCooldown;
            }
        }

        // ! warp handling
        if (input.warp)
        {
            handleWarp(entity, deltaTime);
            input.warp = false;
        }

        handlePositionPlayer(entity);

        // ! debug
        printf("PLAYER %d | Stamina: %.2f | WarpCD: %.2f\n", playerComp.playerID, playerComp.stamina,
               playerComp.warpCooldown);
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
    if (!entity->hasComponent<TransformComponent>() || !entity->hasComponent<InputComponent>() ||
        !entity->hasComponent<PlayerComponent>())
        return;

    auto &transform = entity->getComponent<TransformComponent>();
    auto &input = entity->getComponent<InputComponent>();
    auto &playerComp = entity->getComponent<PlayerComponent>();

    // ! check cooldown and stamina
    if (playerComp.warpCooldown > 0.0f)
        return;
    if (playerComp.stamina < WARP_STAMINA_COST)
        return;

    // ! execute warp
    if (input.up)
        transform.position.y -= WARP_DISTANCE;
    else if (input.down)
        transform.position.y += WARP_DISTANCE;
    else if (input.left)
        transform.position.x -= WARP_DISTANCE;
    else if (input.right)
        transform.position.x += WARP_DISTANCE;

    // ! apply stamina + cooldown
    playerComp.stamina -= WARP_STAMINA_COST;
    playerComp.warpCooldown = WARP_COOLDOWN;

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

    auto &player = entity->getComponent<PlayerComponent>(); // ! was copy, now reference

    printf("BULLET OWNER ID: %d\n", player.playerID);

    bullet.addComponent<ProjectileComponent>(30.0f, 2.0f, player.playerID, ENTITY_TYPE::PLAYER);
}