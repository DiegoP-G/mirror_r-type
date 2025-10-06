#pragma once

#include "../components/ColliderComponent.hpp"
#include "../components/EnemyComponent.hpp"
#include "../components/ProjectileComponent.hpp"
#include "../components/TransformComponent.hpp"
#include "../entityManager.hpp"

#include "../components/CenteredComponent.hpp"
#include "../components/ColliderComponent.hpp"
#include "../components/EnemyComponent.hpp"
#include "../components/HealthComponent.hpp"
#include "../components/JumpComponent.hpp"
#include "../components/LaserWarningComponent.hpp"
#include "../components/PlayerComponent.hpp"
#include "../components/ProjectileComponent.hpp"
#include "../components/Rectangle.hpp"
#include "../components/TransformComponent.hpp"
#include "../components/Vector2D.hpp"

inline Vector2D getActualPosition(Entity *entity)
{
    auto &transform = entity->getComponent<TransformComponent>();
    Vector2D position = transform.position;

    // Utiliser hasComponent au lieu d'un try-catch
    if (entity->hasComponent<CenteredComponent>())
    {
        auto &centered = entity->getComponent<CenteredComponent>();
        position.x -= centered.offsetX;
        position.y -= centered.offsetY;
    }
    return position;
}

class CollisionSystem
{
  public:
    void update(EntityManager &entityManager)
    {
        auto entities = entityManager.getEntitiesWithComponents<TransformComponent, ColliderComponent>();

        for (size_t i = 0; i < entities.size(); i++)
        {
            if (!entities[i]->isActive())
                continue;

            auto &collider1 = entities[i]->getComponent<ColliderComponent>();
            if (!collider1.isActive)
                continue;

            // Update hitbox position
            Vector2D position1 = getActualPosition(entities[i]);
            collider1.hitbox.x = position1.x;
            collider1.hitbox.y = position1.y;

            for (size_t j = i + 1; j < entities.size(); j++)
            {
                if (!entities[j]->isActive())
                    continue;

                auto &collider2 = entities[j]->getComponent<ColliderComponent>();
                if (!collider2.isActive)
                    continue;

                // Update hitbox position
                Vector2D position2 = getActualPosition(entities[j]);
                collider2.hitbox.x = position2.x;
                collider2.hitbox.y = position2.y;

                // Check collision
                if (collider1.hitbox.intersects(collider2.hitbox))
                {
                    handleCollision(entities[i], entities[j]);
                }
            }
        }
    }

  private:
    // ============================================================
    // COLLISION DETECTION HELPERS
    // ============================================================

    enum class EntityType
    {
        PLAYER,
        ENEMY,
        PROJECTILE,
        LASER_WARNING,
        JUMPER,
        UNKNOWN
    };

    EntityType getEntityType(Entity *entity) const
    {
        if (entity->hasComponent<PlayerComponent>())
            return EntityType::PLAYER;
        if (entity->hasComponent<EnemyComponent>())
            return EntityType::ENEMY;
        if (entity->hasComponent<ProjectileComponent>())
            return EntityType::PROJECTILE;
        if (entity->hasComponent<LaserWarningComponent>())
            return EntityType::LASER_WARNING;
        if (entity->hasComponent<JumpComponent>())
            return EntityType::JUMPER;
        return EntityType::UNKNOWN;
    }

    // ============================================================
    // MAIN COLLISION HANDLER
    // ============================================================

    void handleCollision(Entity *a, Entity *b)
    {
        EntityType typeA = getEntityType(a);
        EntityType typeB = getEntityType(b);

        // Sort entities so we always handle collisions in the same order
        if (typeA > typeB)
        {
            std::swap(a, b);
            std::swap(typeA, typeB);
        }

        // Route to specific collision handlers
        switch (typeA)
        {
        case EntityType::PLAYER:
            handlePlayerCollision(a, b, typeB);
            break;

        case EntityType::ENEMY:
            handleEnemyCollision(a, b, typeB);
            break;

        case EntityType::PROJECTILE:
            handleProjectileCollision(a, b, typeB);
            break;

        case EntityType::LASER_WARNING:
            handleLaserCollision(a, b, typeB);
            break;

        case EntityType::JUMPER:
            handleJumperCollision(a, b, typeB);
            break;

        default:
            break;
        }
    }

    // ============================================================
    // PLAYER COLLISION HANDLERS
    // ============================================================

    void handlePlayerCollision(Entity *player, Entity *other, EntityType otherType)
    {
        switch (otherType)
        {
        case EntityType::ENEMY:
            // onPlayerHitEnemy(player, other);
            break;

        case EntityType::PROJECTILE:
            onPlayerHitProjectile(player, other);
            break;

        case EntityType::LASER_WARNING:
            onPlayerHitLaser(player, other);
            break;

        default:
            break;
        }
    }

    // void onPlayerHitEnemy(Entity *player, Entity *enemy)
    // {
    //     std::cout << "[Collision] Player hit by enemy!" << std::endl;
    //
    //     if (player->hasComponent<HealthComponent>())
    //     {
    //         auto &health = player->getComponent<HealthComponent>();
    //         health.health -= 10.0f;
    //
    //         if (health.health <= 0)
    //         {
    //             player->destroy();
    //             std::cout << "[Collision] Player destroyed!" << std::endl;
    //         }
    //     }
    //     else
    //     {
    //         player->destroy();
    //     }
    // }

    void onPlayerHitProjectile(Entity *player, Entity *projectile)
    {
        auto &projComp = projectile->getComponent<ProjectileComponent>();

        // Don't hit yourself
        if (player->getID() == projComp.owner)
            return;

        // std::cout << "[Collision] Player hit by projectile!" << std::endl;

        // if (player->hasComponent<HealthComponent>())
        // {
        //     auto &health = player->getComponent<HealthComponent>();
        //     health.health -= projComp.damage;
        //
        //     if (health.health <= 0)
        //     {
        //         player->destroy();
        //         std::cout << "[Collision] Player destroyed by projectile!" << std::endl;
        //     }
        // }
        // else
        // {
        //     player->destroy();
        // }

        projectile->destroy();
    }

    void onPlayerHitLaser(Entity *player, Entity *laser)
    {
        auto &laserComp = laser->getComponent<LaserWarningComponent>();

        // Only damage if laser is active (not in warning phase)
        if (!laserComp.isActive || laserComp.appearanceTime > 0.0f)
            return;

        // std::cout << "[Collision] Player hit by active laser!" << std::endl;

        // if (player->hasComponent<HealthComponent>())
        // {
        //     auto &health = player->getComponent<HealthComponent>();
        //     health.health -= 50.0f; // Heavy laser damage
        //
        //     if (health.health <= 0)
        //     {
        //         player->destroy();
        //         std::cout << "[Collision] Player destroyed by laser!" << std::endl;
        //     }
        // }
        // else
        // {
        //     player->destroy();
        //     std::cout << "[Collision] Player destroyed by laser (no health)!" << std::endl;
        // }
    }

    // ============================================================
    // ENEMY COLLISION HANDLERS
    // ============================================================

    void handleEnemyCollision(Entity *enemy, Entity *other, EntityType otherType)
    {
        switch (otherType)
        {
        case EntityType::PROJECTILE:
            onEnemyHitProjectile(enemy, other);
            break;

        default:
            break;
        }
    }

    void onEnemyHitProjectile(Entity *enemy, Entity *projectile)
    {
        auto &projComp = projectile->getComponent<ProjectileComponent>();

        // Don't hit yourself (enemy shooting itself)
        if (enemy->getID() == projComp.owner)
            return;

        std::cout << "[Collision] Enemy hit by projectile!" << std::endl;

        // Damage enemy
        if (enemy->hasComponent<HealthComponent>())
        {
            auto &health = enemy->getComponent<HealthComponent>();
            health.health -= projComp.damage;

            if (health.health <= 0)
            {
                enemy->destroy();
                std::cout << "[Collision] Enemy destroyed!" << std::endl;
            }
        }
        else
        {
            // No health component = instant death
            enemy->destroy();
            std::cout << "[Collision] Enemy destroyed (no health)!" << std::endl;
        }

        // Destroy projectile
        projectile->destroy();
    }

    // ============================================================
    // PROJECTILE COLLISION HANDLERS
    // ============================================================

    void handleProjectileCollision(Entity *projectile, Entity *other, EntityType otherType)
    {
        // Already handled in reverse order
        // (e.g., PLAYER vs PROJECTILE handles both)
        if (otherType == EntityType::LASER_WARNING)
        {
            onProjectileHitLaser(projectile, other);
        }
    }

    void onProjectileHitLaser(Entity *projectile, Entity *laser)
    {
        auto &laserComp = laser->getComponent<LaserWarningComponent>();

        // Can only destroy laser if it's still in warning phase
        if (!laserComp.isActive && laserComp.warningShown)
        {
            std::cout << "[Collision] Laser warning destroyed by projectile!" << std::endl;
            laser->destroy();
            projectile->destroy();
        }
    }

    // ============================================================
    // LASER COLLISION HANDLERS
    // ============================================================

    void handleLaserCollision(Entity *laser, Entity *other, EntityType otherType)
    {
        // Already handled in reverse order
        // (e.g., PLAYER vs LASER handles both)
    }

    // ============================================================
    // JUMPER COLLISION HANDLERS (Flappy Bird)
    // ============================================================

    void handleJumperCollision(Entity *jumper, Entity *other, EntityType otherType)
    {
        // Jumper hits anything = game over
        std::cout << "[Collision] Jumper hit something!" << std::endl;
        jumper->destroy();
    }
};