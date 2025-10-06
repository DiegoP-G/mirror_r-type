#include "EnemySystem.hpp"

void EnemySystem::update(EntityManager &entityManager, float deltaTime)
{
    auto entities = entityManager.getEntitiesWithComponents<EnemyComponent, TransformComponent>();

    for (auto &entity : entities)
    {
        auto &enemy = entity->getComponent<EnemyComponent>();
        auto &transform = entity->getComponent<TransformComponent>();

        switch (enemy.type)
        {
        case 0:
            if (entity->hasComponent<VelocityComponent>())
            {
                entity->getComponent<VelocityComponent>().velocity = Vector2D(-50.0f, 0);
            }
            break;

        case 1:
            if (entity->hasComponent<VelocityComponent>())
            {
                static float time = 0;
                time += deltaTime;
                entity->getComponent<VelocityComponent>().velocity = Vector2D(-50.0f, sinf(time * 1.0f) * 500.0f);
            }
            break;
        }
        if (enemy.currentCooldown > 0)
        {
            enemy.currentCooldown -= deltaTime;
        }
        else
        {
            enemy.currentCooldown = enemy.attackCooldown;
            enemyFire(entityManager, entity);
        }

        if (transform.position.x < -50.0f)
        {
            entity->destroy();
        }
    }
}

void EnemySystem::enemyFire(EntityManager &entityManager, Entity *enemy)
{
    auto &enemyComponent = enemy->getComponent<EnemyComponent>();
    if (enemyComponent.shootingType == 1)
    {
        auto &transform = enemy->getComponent<TransformComponent>();

        auto &projectile = entityManager.createEntity();

        projectile.addComponent<TransformComponent>(
            transform.position.x - 20.0f,
            transform.position.y + 10.0f);

        projectile.addComponent<VelocityComponent>(-200.0f, 0.0f);
        projectile.addComponent<SpriteComponent>(10.0f, 5.0f, 255, 0, 0, GraphicsManager::Texture::BULLET);
        projectile.addComponent<ColliderComponent>(10.0f, 5.0f);
        projectile.addComponent<ProjectileComponent>(5.0f, 3.0f, enemy->getID());
    }
    else if (enemyComponent.shootingType == 2)
    {
        auto &transform = enemy->getComponent<TransformComponent>();

        for (int i = 0; i < 3; i++)
        {
            auto &projectile = entityManager.createEntity();
            projectile.addComponent<TransformComponent>(
                transform.position.x - 20.0f,
                transform.position.y + 10.0f);
            projectile.addComponent<SpriteComponent>(10.0f, 5.0f, 255, 255, 255, GraphicsManager::Texture::BULLET);
            projectile.addComponent<VelocityComponent>(-200.0f, (i - 1) * 50.0f);
            projectile.addComponent<ColliderComponent>(10.0f, 5.0f);
            projectile.addComponent<ProjectileComponent>(5.0f, 3.0f, enemy->getID());
        }
    }
}
