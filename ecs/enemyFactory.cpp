#include "enemyFactory.hpp"
#include "systems/EnemySystem.hpp"

void EnemyFactory::createEnemy(EntityManager &entityManager, std::string enemyType, const Vector2D &position, SHOOTINGTYPE shootingType)
{
    auto &enemy = entityManager.createEntity();
    // 0 for movement type : left
    enemy.addComponent<EnemyComponent>(0, 1.0f, shootingType, 50);

    if (enemyType == "basic_enemy")
    {
        enemy.addComponent<ColliderComponent>(20.0f, 20.0f, true);
        enemy.addComponent<HealthComponent>(100, 100);
        enemy.addComponent<HealthBarComponent>(30.0f, 4.0f, 14.0f, -16.0f);
        enemy.addComponent<TransformComponent>(position.x, position.y);
        enemy.addComponent<VelocityComponent>(0.0f, 0.0f);
        enemy.addComponent<AnimatedSpriteComponent>(GraphicsManager::Texture::ENEMY, 98.5, 32.3, 32.8, 32.3, 1, 0.05f,
                                                    -90.0f);
    }
    else if (enemyType == "boss")
    {
        enemy.addComponent<ColliderComponent>(100.0f, 100.0f, true);
        enemy.addComponent<HealthComponent>(200, 200);
        enemy.addComponent<HealthBarComponent>(60.0f, 4.0f, -90.0f);
        enemy.addComponent<TransformComponent>(position.x - 70, position.y);
        enemy.addComponent<AnimatedSpriteComponent>(GraphicsManager::Texture::BOSS, 0, 32, 110, 112, 1, 0.05f, 0.0f,
                                                    Vector2D(1.5f, 1.5f));
    }
}