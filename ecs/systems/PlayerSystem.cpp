#include "PlayerSystem.hpp"

void PlayerSystem::update(EntityManager &entityManager, float deltaTime, bool client)
{
    auto entities = entityManager.getEntitiesWithComponents<InputComponent, PlayerComponent>();

    for (auto &entity : entities)
    {
        auto &input = entity->getComponent<InputComponent>();
        auto &player = entity->getComponent<PlayerComponent>();
        player.currentCooldown -= deltaTime;

        if (player.stamina > player.maxStamina)
            player.stamina = player.maxStamina;

        if (input.warp && player.stamina >= WARP_STAMINA_COST * deltaTime)
        {
            player.stamina -= WARP_STAMINA_COST * deltaTime;
            if (player.stamina < 0.0f)
                player.stamina = 0.0f;

            player.moveSpeed = NORMAL_SPEED * WARP_SPEED_MULTIPLIER;
        }
        else
        {
            player.moveSpeed = NORMAL_SPEED;
            player.stamina += player.staminaRegenRate * deltaTime;
            if (player.stamina > player.maxStamina)
                player.stamina = player.maxStamina;
        }

        if (input.fire && player.currentCooldown <= 0)
        {
            if (player.stamina >= FIRE_STAMINA_COST)
            {
                if (!client)
                    fire(entityManager, entity);

                player.stamina -= FIRE_STAMINA_COST;
                input.fire = false;
                player.currentCooldown = player.attackCooldown;
            }
        }

        if (entity->hasComponent<TransformComponent>())
        {
            auto &transform = entity->getComponent<TransformComponent>();
            if (input.up)
                transform.position.y -= player.moveSpeed * deltaTime;
            if (input.down)
                transform.position.y += player.moveSpeed * deltaTime;
            if (input.left)
                transform.position.x -= player.moveSpeed * deltaTime;
            if (input.right)
                transform.position.x += player.moveSpeed * deltaTime;

            handlePositionPlayer(entity);
        }

        printf("PLAYER %d | Stamina: %.2f | Speed: %.2f\n", player.playerID, player.stamina, player.moveSpeed);
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
    auto &bullet = entityManager.createEntity();

    bullet.addComponent<TransformComponent>(transform.position.x + 32.0f, transform.position.y + 16.0f);
    bullet.addComponent<VelocityComponent>(300.0f, 0.0f);
    bullet.addComponent<SpriteComponent>(8, 8, 255, 0, 0);
    bullet.addComponent<ColliderComponent>(8.0f, 8.0f);

    auto &player = entity->getComponent<PlayerComponent>();
    printf("BULLET OWNER ID: %d\n", player.playerID);

    bullet.addComponent<ProjectileComponent>(30.0f, 2.0f, player.playerID, ENTITY_TYPE::PLAYER);
}
