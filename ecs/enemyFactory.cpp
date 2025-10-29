#include "enemyFactory.hpp"
#include "systems/EnemySystem.hpp"

std::pair<float, float> computeHitbox(float width, float height, float scaleX, float scaleY, float rotation)
{
    float scaledWidth = width * scaleX;
    float scaledHeight = height * scaleY;

    float radians = rotation * (M_PI / 180.0f); // Convert degrees to radians
    float rotatedWidth = std::abs(scaledWidth * std::cos(radians)) + std::abs(scaledHeight * std::sin(radians));
    float rotatedHeight = std::abs(scaledWidth * std::sin(radians)) + std::abs(scaledHeight * std::cos(radians));

    return {rotatedWidth, rotatedHeight};
}

void EnemyFactory::createEnemy(EntityManager &entityManager, std::string enemyType, const Vector2D &position,
                               SHOOTINGTYPE shootingType)
{
    auto &enemy = entityManager.createEntity();
    // 0 for movement type : left
    enemy.addComponent<EnemyComponent>(0, 1.0f, shootingType, 50);

    const EnemyProperties *properties = nullptr;

    if (enemyType == "basic_enemy")
    {
        properties = &BASIC_ENEMY;
    }
    else if (enemyType == "rotating_enemy")
    {
        properties = &ROTATING_ENEMY;
    }
    else if (enemyType == "boss")
    {
        properties = &BOSS;
    }

    if (properties)
    {
        auto hitbox = computeHitbox(properties->width, properties->height, properties->scaleX, properties->scaleY,
                                    properties->rotation);

        enemy.addComponent<ColliderComponent>(hitbox.first, hitbox.second, true);
        enemy.addComponent<HealthComponent>(properties->health, properties->health);
        enemy.addComponent<HealthBarComponent>(30.0f, 4.0f, 14.0f, -16.0f);
        enemy.addComponent<TransformComponent>(position.x, position.y);
        enemy.addComponent<VelocityComponent>(0.0f, 0.0f);
        enemy.addComponent<AnimatedSpriteComponent>(properties->texture, properties->top, properties->left,
                                                    properties->width, properties->height, properties->numFrames,
                                                    properties->interval, properties->rotation, properties->layout,
                                                    Vector2D(properties->scaleX, properties->scaleY));
    }
}