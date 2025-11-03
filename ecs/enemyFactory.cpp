#include "enemyFactory.hpp"
#include "systems/EnemySystem.hpp"

#ifdef _WIN32
#define M_PI 3.14159265358979323846
#endif

std::pair<float, float> computeHitbox(float width, float height, float scaleX, float scaleY, float rotation)
{
    float scaledWidth = width * scaleX;
    float scaledHeight = height * scaleY;

    float radians = rotation * (M_PI / 180.0f);
    float rotatedWidth = std::abs(scaledWidth * std::cos(radians)) + std::abs(scaledHeight * std::sin(radians));
    float rotatedHeight = std::abs(scaledWidth * std::sin(radians)) + std::abs(scaledHeight * std::cos(radians));

    return {rotatedWidth, rotatedHeight};
}

std::tuple<float, float, float, float> computeHealthBar(const EnemyProperties &properties)
{
    float healthBarWidth = std::max(30.0f, properties.health * 0.2f);
    float healthBarHeight = 4.0f;

    float effectiveWidth = properties.width * properties.scaleX;
    float effectiveHeight = properties.height * properties.scaleY;

    if (std::abs(properties.rotation) == 90.0f)
    {
        std::swap(effectiveWidth, effectiveHeight);
    }

    float offsetX = (effectiveWidth - healthBarWidth) / 2.0f;

    float offsetY = effectiveHeight + properties.top;

    return {healthBarWidth, healthBarHeight, offsetX, offsetY};
}

void EnemyFactory::createEnemy(EntityManager &entityManager, std::string enemyType, const Vector2D &position,
                               SHOOTINGTYPE shootingType, MOVEMENTTYPE movement)
{
    auto &enemy = entityManager.createEntity();
    enemy.addComponent<EnemyComponent>(movement, 1.0f, shootingType, 50);

    const EnemyProperties *properties = nullptr;

    if (enemyType == "basic_enemy")
    {
        properties = &BASIC_ENEMY;
    }
    else if (enemyType == "rotating_enemy")
    {
        properties = &ROTATING_ENEMY;
    }
    else if (enemyType == "purple_enemy")
    {
        properties = &PURPLE_ENEMY;
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

        auto [healthBarWidth, healthBarHeight, offsetX, offsetY] = computeHealthBar(*properties);

        if (enemyType == "basic_enemy")
        {
            enemy.addComponent<HealthBarComponent>(healthBarWidth, healthBarHeight, 20, 0);
        }
        else if (enemyType == "rotating_enemy")
        {
            enemy.addComponent<HealthBarComponent>(healthBarWidth, healthBarHeight, 20, 0);
        }
        else if (enemyType == "purple_enemy")
        {
            enemy.addComponent<HealthBarComponent>(healthBarWidth, healthBarHeight, 30, 0);
        }
        else if (enemyType == "boss")
        {
            enemy.addComponent<HealthBarComponent>(healthBarWidth, healthBarHeight, 90, 0);
        }
        enemy.addComponent<TransformComponent>(position.x, position.y);
        if (enemyType == "boss")
            enemy.addComponent<VelocityComponent>(0.0f, -30.0f);
        else
            enemy.addComponent<VelocityComponent>(0.0f, 0.0f);
        enemy.addComponent<AnimatedSpriteComponent>(properties->texture, properties->top, properties->left,
                                                    properties->width, properties->height, properties->numFrames,
                                                    properties->interval, properties->rotation, properties->layout,
                                                    Vector2D(properties->scaleX, properties->scaleY));
    }
}