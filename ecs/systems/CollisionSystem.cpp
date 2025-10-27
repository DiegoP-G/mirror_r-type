#include "CollisionSystem.hpp"
#include <iostream>

void CollisionSystem::update(EntityManager &entityManager, std::vector<std::pair<int, int>> &playersScores,
                             bool &updateScore)
{
    auto entities = entityManager.getEntitiesWithComponents<TransformComponent, ColliderComponent>();

    for (size_t i = 0; i < entities.size(); i++)
    {
        if (!entities[i]->isActive())
            continue;

        auto &collider1 = entities[i]->getComponent<ColliderComponent>();
        if (!collider1.isActive)
            continue;

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

            Vector2D position2 = getActualPosition(entities[j]);
            collider2.hitbox.x = position2.x;
            collider2.hitbox.y = position2.y;

            if (collider1.hitbox.intersects(collider2.hitbox))
            {
                handleCollision(entities[i], entities[j], playersScores, updateScore, entityManager);
            }
        }
    }
}

CollisionSystem::EntityType CollisionSystem::getEntityType(Entity *entity) const
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
    if (entity->hasComponent<BonusComponent>())
        return EntityType::BONUS;
    return EntityType::UNKNOWN;
}

void CollisionSystem::handleCollision(Entity *a, Entity *b, std::vector<std::pair<int, int>> &playersScores,
                                      bool &updateScore, EntityManager &entityManager)
{
    EntityType typeA = getEntityType(a);
    EntityType typeB = getEntityType(b);

    if (typeA > typeB)
    {
        std::swap(a, b);
        std::swap(typeA, typeB);
    }

    switch (typeA)
    {
    case EntityType::PLAYER:
        handlePlayerCollision(a, b, typeB);
        break;

    case EntityType::ENEMY:
        handleEnemyCollision(a, b, typeB, playersScores, updateScore, entityManager);
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

void CollisionSystem::handlePlayerCollision(Entity *player, Entity *other, EntityType otherType)
{
    switch (otherType)
    {
    case EntityType::PROJECTILE:
        onPlayerHitProjectile(player, other);
        break;

    case EntityType::LASER_WARNING:
        onPlayerHitLaser(player, other);
        break;

    case EntityType::BONUS:
        onPlayerHitBonus(player, other);
        break;

    default:
        break;
    }
}

void CollisionSystem::onPlayerHitProjectile(Entity *player, Entity *projectile)
{
    auto &projComp = projectile->getComponent<ProjectileComponent>();

    if (player->getID() == projComp.owner_id)
        return;

    if (projComp.owner_type == PLAYER)
        return;

    if (player->hasComponent<HealthComponent>())
    {
        auto &health = player->getComponent<HealthComponent>();
        health.health -= projComp.damage;

        if (health.health <= 0)
        {
            player->destroy();
        }
    }

    // std::cout << "-------------porjectile hit---------------" << std::endl;
    projectile->destroy();
}

void CollisionSystem::onPlayerHitBonus(Entity *player, Entity *bonus)
{
    auto &BonusComp = bonus->getComponent<BonusComponent>();

    auto &healthComp = player->getComponent<HealthComponent>();

    for (auto &b : BonusComp.bonus)
    {
        if (std::get<0>(b) == BonusComponent::TypeBonus::HEALTH)
        {
            healthComp.health += std::get<1>(b);
            if (healthComp.health > healthComp.maxHealth)
                healthComp.health = healthComp.maxHealth;
        }
    }
    std::cout << "BONUS DESTOYED HIHI\n";
    bonus->destroy();
}

void CollisionSystem::onPlayerHitLaser(Entity *player, Entity *laser)
{
    auto &laserComp = laser->getComponent<LaserWarningComponent>();

    if (!laserComp.isActive || laserComp.appearanceTime > 0.0f)
        return;
}

void CollisionSystem::handleEnemyCollision(Entity *enemy, Entity *other, EntityType otherType,
                                           std::vector<std::pair<int, int>> &playersScores, bool &updateScore, EntityManager &entityManager)
{
    switch (otherType)
    {
    case EntityType::PROJECTILE:
        onEnemyHitProjectile(enemy, other, playersScores, updateScore, entityManager);
        break;

    default:
        break;
    }
}

void CollisionSystem::onEnemyHitProjectile(Entity *enemy, Entity *projectile,
                                           std::vector<std::pair<int, int>> &playersScores, bool &updateScore, EntityManager &entityManager)
{
    auto &projComp = projectile->getComponent<ProjectileComponent>();

    if (enemy->getID() == projComp.owner_id)
        return;

    if (projComp.owner_type == ENEMY)
        return;

    EnemyComponent &enemyComp = enemy->getComponent<EnemyComponent>();

    if (enemy->hasComponent<HealthComponent>())
    {
        auto &health = enemy->getComponent<HealthComponent>();
        health.health -= projComp.damage;

        if (health.health <= 0)
        {
            enemy->destroy();

            auto &explosion = entityManager.createEntity();
            auto &transform = enemy->getComponent<TransformComponent>();
            explosion.addComponent<TransformComponent>(transform.position.x, transform.position.y);
            explosion.addComponent<AnimatedSpriteComponent>(GraphicsManager::Texture::EXPLOSION,
                282, 296, 36, 36, 5, 0.05f, 0.0f, Vector2D(1.0f, 1.0f), 0, true);

            for (auto &pair : playersScores)
            {
                if (pair.first == projComp.owner_id)
                {
                    pair.second += enemyComp.scoreValue;
                    updateScore = true;
                }
            }
        }
    }
    else
    {
        enemy->destroy();
        for (auto &pair : playersScores)
        {
            if (pair.first == projComp.owner_id)
            {
                pair.second += enemyComp.scoreValue;
                updateScore = true;
            }
        }
    }
    projectile->destroy();
}

void CollisionSystem::handleProjectileCollision(Entity *projectile, Entity *other, EntityType otherType)
{
    if (otherType == EntityType::LASER_WARNING)
    {
        onProjectileHitLaser(projectile, other);
    }
}

void CollisionSystem::onProjectileHitLaser(Entity *projectile, Entity *laser)
{
    auto &laserComp = laser->getComponent<LaserWarningComponent>();

    if (!laserComp.isActive && laserComp.warningShown)
    {
        // std::cout << "[Collision] Laser warning destroyed by projectile!" << std::endl;
        laser->destroy();
        projectile->destroy();
    }
}

void CollisionSystem::handleLaserCollision(Entity *laser, Entity *other, EntityType otherType)
{
    // Already handled in reverse order
}

void CollisionSystem::handleJumperCollision(Entity *jumper, Entity *other, EntityType otherType)
{
    //  std::cout << "[Collision] Jumper hit something!" << std::endl;
    jumper->destroy();
}
