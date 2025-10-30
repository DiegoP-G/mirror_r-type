#include "PlayerSystem.hpp"

void PlayerSystem::update(EntityManager &entityManager, float deltaTime, bool client)
{
    auto entities = entityManager.getEntitiesWithComponents<InputComponent, PlayerComponent>();

    for (auto &entity : entities)
    {
        auto &input = entity->getComponent<InputComponent>();
        auto &playerComp = entity->getComponent<PlayerComponent>();
        playerComp.currentCooldown -= deltaTime;
        playerComp.bonusFiremode -= deltaTime;
        playerComp.bonusShield -= deltaTime;

        auto shieldEntities = entityManager.getEntitiesWithComponent<ShieldComponent>();
        for (auto &shieldEntity : shieldEntities)
        {
            auto &shieldComp = shieldEntity->getComponent<ShieldComponent>();
            if (shieldComp.ownerID == playerComp.playerID)
            {
                shieldComp.shieldLeft -= deltaTime;
                break;
            }
        }

        if (playerComp.stamina > playerComp.maxStamina)
            playerComp.stamina = playerComp.maxStamina;

        if (input.warp && playerComp.stamina >= WARP_STAMINA_COST * deltaTime)
        {
            playerComp.stamina -= WARP_STAMINA_COST * deltaTime;
            if (playerComp.stamina < 0.0f)
                playerComp.stamina = 0.0f;

            playerComp.moveSpeed = NORMAL_SPEED * WARP_SPEED_MULTIPLIER;
        }
        else
        {
            playerComp.moveSpeed = NORMAL_SPEED;
            playerComp.stamina += playerComp.staminaRegenRate * deltaTime;
            if (playerComp.stamina > playerComp.maxStamina)
                playerComp.stamina = playerComp.maxStamina;
        }

        if (input.fire && playerComp.currentCooldown <= 0)
        {
            if (playerComp.stamina >= FIRE_STAMINA_COST)
            {
                if (!client)
                    fire(entityManager, entity);

                playerComp.stamina -= FIRE_STAMINA_COST;
                input.fire = false;
                playerComp.currentCooldown = playerComp.attackCooldown;
            }
        }

        if (entity->hasComponent<TransformComponent>())
        {
            auto &transform = entity->getComponent<TransformComponent>();
            if (input.up)
                transform.position.y -= playerComp.moveSpeed * deltaTime;
            if (input.down)
                transform.position.y += playerComp.moveSpeed * deltaTime;
            if (input.left)
                transform.position.x -= playerComp.moveSpeed * deltaTime;
            if (input.right)
                transform.position.x += playerComp.moveSpeed * deltaTime;

            handlePositionPlayer(entity);
        }

        printf("PLAYER %d | Stamina: %.2f | Speed: %.2f\n", playerComp.playerID, playerComp.stamina,
               playerComp.moveSpeed);
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

void PlayerSystem::fire(EntityManager &entityManager, Entity *entity)
{
    auto &transform = entity->getComponent<TransformComponent>();
    auto &player = entity->getComponent<PlayerComponent>();

    if (player.bonusFiremode > 0)
    {
        for (int i = 0; i < 3; i++)
        {
            auto &bullet = entityManager.createEntity();

            bullet.addComponent<TransformComponent>(transform.position.x + 32.0f, transform.position.y + 16.0f);

            Vector2D velocity(300.0f, (i - 1) * 50.0f);
            bullet.addComponent<VelocityComponent>(velocity.x, velocity.y);

            // float angle = std::atan2(velocity.y, velocity.x) * (180.0f / M_PI) + 90.0f;
            // bullet.addComponent<AnimatedSpriteComponent

            bullet.addComponent<SpriteComponent>(8, 8, 255, 0, 0);
            bullet.addComponent<ColliderComponent>(8.0f, 8.0f);
            bullet.addComponent<ProjectileComponent>(30.0f, 2.0f, player.playerID, ENTITY_TYPE::PLAYER);
        }
    }
    else
    {
        auto &bullet = entityManager.createEntity();

        bullet.addComponent<TransformComponent>(transform.position.x + 32.0f, transform.position.y + 16.0f);
        bullet.addComponent<VelocityComponent>(300.0f, 0.0f);
        bullet.addComponent<SpriteComponent>(8, 8, 255, 0, 0);
        bullet.addComponent<ColliderComponent>(8.0f, 8.0f);
        bullet.addComponent<ProjectileComponent>(30.0f, 2.0f, player.playerID, ENTITY_TYPE::PLAYER);
    }
    printf("BULLET OWNER ID: %d\n", player.playerID);
}
