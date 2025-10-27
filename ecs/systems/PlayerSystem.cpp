#include "PlayerSystem.hpp"

void PlayerSystem::update(EntityManager &entityManager, float deltaTime)
{
    auto entities = entityManager.getEntitiesWithComponents<InputComponent, PlayerComponent>();

    for (auto &entity : entities)
    {
        auto &input = entity->getComponent<InputComponent>();
        auto &playerComp = entity->getComponent<PlayerComponent>();
        playerComp.currentCooldown -= deltaTime;
        playerComp.bonusFiremode -= deltaTime;
        if (input.fire && playerComp.currentCooldown <= 0)
        {
            fire(entityManager, entity);
            input.fire = false;
            playerComp.currentCooldown = playerComp.attackCooldown;
        }

        handlePositionPlayer(entity);
    }
}

void PlayerSystem::handlePositionPlayer(Entity *&entity)
{
    if (!entity->hasComponent<TransformComponent>())
        return;
    auto &tranform = entity->getComponent<TransformComponent>();
    if (tranform.position.x > 800 - 32)
        tranform.position.x = 800 - 32;

    if (tranform.position.x < 0)
        tranform.position.x = 0;

    if (tranform.position.y < 0)
        tranform.position.y = 0;

    if (tranform.position.y > 600 - 32)
        tranform.position.y = 600 - 32;
}

void PlayerSystem::fire(EntityManager &entityManager, Entity *entity)
{
    auto &transform = entity->getComponent<TransformComponent>();
    auto &player = entity->getComponent<PlayerComponent>();

    if (player.bonusFiremode > 0) {
        for (int i = 0; i < 3; i++) {
            auto &bullet = entityManager.createEntity();
            
            bullet.addComponent<TransformComponent>(transform.position.x + 32.0f, transform.position.y + 16.0f);
            
            Vector2D velocity(300.0f, (i - 1) * 50.0f);
            bullet.addComponent<VelocityComponent>(velocity.x, velocity.y);

            //float angle = std::atan2(velocity.y, velocity.x) * (180.0f / M_PI) + 90.0f;
            //bullet.addComponent<AnimatedSpriteComponent

            bullet.addComponent<SpriteComponent>(8, 8, 255, 0, 0);
            bullet.addComponent<ColliderComponent>(8.0f, 8.0f);
            bullet.addComponent<ProjectileComponent>(30.0f, 2.0f, player.playerID, ENTITY_TYPE::PLAYER);
        }
    } else {
        auto &bullet = entityManager.createEntity();

        bullet.addComponent<TransformComponent>(transform.position.x + 32.0f, transform.position.y + 16.0f);
        bullet.addComponent<VelocityComponent>(300.0f, 0.0f);
        bullet.addComponent<SpriteComponent>(8, 8, 255, 0, 0);
        bullet.addComponent<ColliderComponent>(8.0f, 8.0f);
        bullet.addComponent<ProjectileComponent>(30.0f, 2.0f, player.playerID, ENTITY_TYPE::PLAYER);
    }
    printf("BULLET OWNER ID: %d\n", player.playerID);
}
