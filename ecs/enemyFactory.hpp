#pragma once
#include "GraphicsManager.hpp"
#include "allComponentsInclude.hpp"
#include "components/HealthBarComponent.hpp"
#include "entityManager.hpp"

class EnemyFactory
{
  public:
    static void createEnemy(EntityManager &entityManager, std::string enemyType, const Vector2D &position)
    {
        auto &enemy = entityManager.createEntity();
        enemy.addComponent<TransformComponent>(position.x, position.y);
        enemy.addComponent<VelocityComponent>(0.0f, 0.0f);
        enemy.addComponent<ColliderComponent>(20.0f, 20.0f, true);
        // 0 for movement type : left
        // 2 for shooting type : single shot
        enemy.addComponent<EnemyComponent>(0, 1.0f, 2);

        enemy.addComponent<HealthComponent>(100, 100);
        enemy.addComponent<HealthBarComponent>(30.0f, 4.0f, -20.0f);

        if (enemyType == "basic_enemy")
        {
            enemy.addComponent<AnimatedSpriteComponent>(GraphicsManager::Texture::ENEMY, 98.5, 32.3, 32.8, 32.3, 1,
                                                        0.05f, -90.0f);
        }
    }
};