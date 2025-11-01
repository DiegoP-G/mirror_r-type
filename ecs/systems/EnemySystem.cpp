#include "EnemySystem.hpp"

#define M_PI 3.14159265358979323846

void EnemySystem::update(EntityManager &entityManager, float deltaTime)
{
    auto entities = entityManager.getEntitiesWithComponents<EnemyComponent, TransformComponent>();

    // std::cerr << "Active Enemies: " << entities.size() << std::endl;
    for (auto &entity : entities)
    {
        auto &enemy = entity->getComponent<EnemyComponent>();
        auto &transform = entity->getComponent<TransformComponent>();

        switch (enemy.type)
        {
        case MOVEMENTTYPE::ONLY_LEFT:
            if (entity->hasComponent<VelocityComponent>())
            {
                entity->getComponent<VelocityComponent>().velocity = Vector2D(-50.0f, 0);
            }
            break;

        case MOVEMENTTYPE::SINE:
            if (entity->hasComponent<VelocityComponent>())
            {
                static float time = 0;
                time += deltaTime;
                entity->getComponent<VelocityComponent>().velocity = Vector2D(-50.0f, sinf(time * 1.0f) * 500.0f);
            }
            break;
        case MOVEMENTTYPE::STATIC_UP_DOWN:
            if (entity->hasComponent<VelocityComponent>())
            {
                auto &velocity = entity->getComponent<VelocityComponent>();
                if (transform.position.y <= 50.0f || transform.position.y >= 550.0f)
                {
                    velocity.velocity.y = -velocity.velocity.y;
                }
                transform.position.y += velocity.velocity.y * deltaTime;
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
            entityManager.markEntityForDestruction(entity->getID());
        }
    }

    auto inactiveEntities = entityManager.getInactiveEntitiesWithComponents<EnemyComponent>();

    for (auto &entity : inactiveEntities)
    {
        if (!entity->isActive())
            entityManager.markEntityForDestruction(entity->getID());
    }
}

void EnemySystem::enemyFire(EntityManager &entityManager, Entity *enemy)
{
    auto &enemyComponent = enemy->getComponent<EnemyComponent>();
    auto &transform = enemy->getComponent<TransformComponent>();

    if (enemyComponent.shootingType == SHOOTINGTYPE::STRAIGHT)
    {
        auto &projectile = entityManager.createEntity();

        projectile.addComponent<TransformComponent>(transform.position.x - 10.0f, transform.position.y);

        Vector2D velocity(-270.0f, 0.0f);
        projectile.addComponent<VelocityComponent>(velocity.x, velocity.y);

        float angle = std::atan2(velocity.y, velocity.x) * (180.0f / M_PI) + 90.0f;
        projectile.addComponent<AnimatedSpriteComponent>(GraphicsManager::Texture::BULLET, redBullet.left,
                                                         redBullet.top, 9, 17, 1, 1, angle);

        projectile.addComponent<ColliderComponent>(10.0f, 5.0f);
        projectile.addComponent<ProjectileComponent>(5.0f, 8.0f, enemy->getID(), ENTITY_TYPE::ENEMY);
    }
    else if (enemyComponent.shootingType == SHOOTINGTYPE::THREE_DISPERSED)
    {
        for (int i = 0; i < 3; i++)
        {
            auto &projectile = entityManager.createEntity();
            projectile.addComponent<TransformComponent>(transform.position.x - 10.0f, transform.position.y);

            Vector2D velocity(-270.0f, (i - 1) * 50.0f);
            projectile.addComponent<VelocityComponent>(velocity.x, velocity.y);

            float angle = std::atan2(velocity.y, velocity.x) * (180.0f / M_PI) + 90.0f;
            projectile.addComponent<AnimatedSpriteComponent>(GraphicsManager::Texture::BULLET, redBullet.left,
                                                             redBullet.top, 9, 17, 1, 1, angle);

            projectile.addComponent<ColliderComponent>(10.0f, 5.0f);
            projectile.addComponent<ProjectileComponent>(5.0f, 8.0f, enemy->getID(), ENTITY_TYPE::ENEMY);
        }
    }
    else if (enemyComponent.shootingType == SHOOTINGTYPE::SINUS)
    {
        auto &projectile1 = entityManager.createEntity();
        projectile1.addComponent<TransformComponent>(transform.position.x - 10.0f, transform.position.y - 15.0f);

        Vector2D velocity1(-270.0f, 0.0f);
        projectile1.addComponent<VelocityComponent>(velocity1.x, velocity1.y, true, false);
        projectile1.addComponent<AnimatedSpriteComponent>(GraphicsManager::Texture::BULLET, redBullet.left,
                                                          redBullet.top, 9, 17, 1, 1, -90.0f);
        projectile1.addComponent<ColliderComponent>(10.0f, 5.0f);
        projectile1.addComponent<ProjectileComponent>(5.0f, 8.0f, enemy->getID(), ENTITY_TYPE::ENEMY);

        auto &projectile2 = entityManager.createEntity();
        projectile2.addComponent<TransformComponent>(transform.position.x - 10.0f, transform.position.y + 15.0f);

        Vector2D velocity2(-200.0f, 0.0f);
        projectile2.addComponent<VelocityComponent>(velocity2.x, velocity2.y, false, true);
        projectile2.addComponent<AnimatedSpriteComponent>(GraphicsManager::Texture::BULLET, redBullet.left,
                                                          redBullet.top, 9, 17, 1, 1, -90.0f);
        projectile2.addComponent<ColliderComponent>(10.0f, 5.0f);
        projectile2.addComponent<ProjectileComponent>(5.0f, 8.0f, enemy->getID(), ENTITY_TYPE::ENEMY);
    }
    else if (enemyComponent.shootingType == SHOOTINGTYPE::ALLDIRECTION)
    {
        const int bulletCount = 12;
        const float speed = 200.0f;

        for (int i = 0; i < bulletCount; ++i)
        {
            float angle = (2.0f * M_PI / bulletCount) * i;
            float vx = speed * cos(angle);
            float vy = speed * sin(angle);

            auto &projectile = entityManager.createEntity();
            projectile.addComponent<TransformComponent>(transform.position.x, transform.position.y);
            projectile.addComponent<VelocityComponent>(vx, vy);
            projectile.addComponent<AnimatedSpriteComponent>(GraphicsManager::Texture::BULLET, redBullet.left,
                                                             redBullet.top, 9, 17, 1, 1, -angle * (180.0f / M_PI) + 45);
            projectile.addComponent<ColliderComponent>(10.0f, 5.0f);
            projectile.addComponent<ProjectileComponent>(5.0f, 8.0f, enemy->getID(), ENTITY_TYPE::ENEMY);
        }
    }
    else if (enemyComponent.shootingType == SHOOTINGTYPE::CIRCLE)
    {
        const int bulletCount = 12;
        const float arcRadius = 100.0f;
        const float angularSpeed = 5.0f * M_PI / 2.0f;

        for (int i = 0; i < bulletCount; ++i)
        {
            float angle = (2.0f * M_PI / bulletCount) * i;

            auto &projectile = entityManager.createEntity();
            projectile.addComponent<TransformComponent>(transform.position.x, transform.position.y);
            projectile.addComponent<AnimatedSpriteComponent>(GraphicsManager::Texture::BULLET, redBullet.left,
                                                             redBullet.top, 9, 17, 1, 1, -angle * (180.0f / M_PI) + 45);
            projectile.addComponent<ColliderComponent>(10.0f, 5.0f);
            projectile.addComponent<ProjectileComponent>(5.0f, 8.0f, enemy->getID(), ENTITY_TYPE::ENEMY);

            Vector2D velocity(-270.0f, 0.0f);
            projectile.addComponent<VelocityComponent>(velocity.x, velocity.y);
            projectile.addComponent<CircularMotionComponent>(arcRadius, angle, angularSpeed);
        }
    }
}
