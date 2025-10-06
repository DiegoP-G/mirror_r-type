#pragma once

#include "../GraphicsManager.hpp"
#include "../components/ColliderComponent.hpp"
#include "../components/EnemyComponent.hpp"
#include "../components/ProjectileComponent.hpp"
#include "../components/SpriteComponent.hpp"
#include "../components/TransformComponent.hpp"
#include "../components/VelocityComponent.hpp"
#include "../entityManager.hpp"

#include <cmath>

class EnemySystem
{
  public:
    void update(EntityManager &entityManager, float deltaTime)
    {
        auto entities = entityManager.getEntitiesWithComponents<EnemyComponent, TransformComponent>();

        for (auto &entity : entities)
        {
            auto &enemy = entity->getComponent<EnemyComponent>();
            auto &transform = entity->getComponent<TransformComponent>();

            // Update enemy behavior based on type
            switch (enemy.type)
            {
            case 0: // Basic enemy - moves left
                if (entity->hasComponent<VelocityComponent>())
                {
                    entity->getComponent<VelocityComponent>().velocity = Vector2D(-50.0f, 0);
                }
                break;

            case 1: // Sine wave movement
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

            // Destroy enemies that go off screen
            if (transform.position.x < -50.0f)
            {
                entity->destroy();
            }
        }
    }

  private:
    void enemyFire(EntityManager &entityManager, Entity *enemy)
    {
        auto &enemyComponent = enemy->getComponent<EnemyComponent>();
        if (enemyComponent.shootingType == 1)
        {

            auto &transform = enemy->getComponent<TransformComponent>();

            // Create projectile entity
            auto &projectile = entityManager.createEntity();

            // Position the projectile at the enemy's position
            projectile.addComponent<TransformComponent>(
                transform.position.x - 20.0f, // Offset to fire from the front of the enemy
                transform.position.y + 10.0f  // Center height
            );

            // Add projectile components
            projectile.addComponent<VelocityComponent>(-200.0f, 0.0f); // Fast horizontal movement
            projectile.addComponent<SpriteComponent>(10.0f, 5.0f, 255, 0, 0, GraphicsManager::Texture::BULLET);
            projectile.addComponent<ColliderComponent>(10.0f, 5.0f);                  // Small hitbox
            projectile.addComponent<ProjectileComponent>(5.0f, 3.0f, enemy->getID()); // Damage, lifetime, owner
        }
        else if (enemyComponent.shootingType == 2)
        {
            auto &transform = enemy->getComponent<TransformComponent>();

            for (int i = 0; i < 3; i++)
            {
                // std::cout << "i = " << i;
                auto &projectile = entityManager.createEntity();
                projectile.addComponent<TransformComponent>(
                    transform.position.x - 20.0f, // Offset to fire from the front of the enemy
                    transform.position.y + 10.0f  // Center height
                );
                projectile.addComponent<SpriteComponent>(10.0f, 5.0f, 255, 255, 255, GraphicsManager::Texture::BULLET);
                projectile.addComponent<VelocityComponent>(-200.0f, (i - 1) * 50.0f); // Spread pattern
                projectile.addComponent<ColliderComponent>(10.0f, 5.0f);
                projectile.addComponent<ProjectileComponent>(5.0f, 3.0f, enemy->getID()); // Damage, lifetime, owner
            }
            // std::cout << std::endl;
        }
    }
};
