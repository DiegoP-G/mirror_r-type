#pragma once
#include "GraphicsManager.hpp"
#include "allComponentsInclude.hpp"
#include "components.hpp"
#include "entityManager.hpp"

class EnemyFactory
{
  public:
    static void createEnemy(EntityManager &entityManager, std::string enemyType, const Vector2D &position)
    {
        auto &enemy = entityManager.createEntity();
        enemy.addComponent<TransformComponent>(position.x, position.y);
        enemy.addComponent<VelocityComponent>(-50.0f, 0.0f);
        enemy.addComponent<ColliderComponent>(20.0f, 20.0f, true);

        if (enemyType == "basic")
        {
            if (auto *texture = g_graphics->getTexture("basic_enemy"))
            {
                enemy.addComponent<AnimatedSpriteComponent>(*texture, 98.5, 32.3, 32.8, 32.3, 1, 1, -90);
            }
            else
            {
                enemy.addComponent<SpriteComponent>(20.0f, 20.0f, 255, 0, 0);
            }
            // 0 for movement type : left
            // 2 for shooting type : single shot
            enemy.addComponent<EnemyComponent>(0, 1.0f, 2);
        }
    }
};