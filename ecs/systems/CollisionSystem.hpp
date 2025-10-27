#pragma once

#include "../components/BonusComponent.hpp"
#include "../components/ColliderComponent.hpp"
#include "../components/EnemyComponent.hpp"
#include "../components/ProjectileComponent.hpp"
#include "../components/TransformComponent.hpp"
#include "../components/AnimatedSpriteComponent.hpp"

#include "../entityManager.hpp"
#include "../GraphicsManager.hpp"

#include "../components/CenteredComponent.hpp"
#include "../components/HealthComponent.hpp"
#include "../components/JumpComponent.hpp"
#include "../components/LaserWarningComponent.hpp"
#include "../components/PlayerComponent.hpp"
#include "../components/Rect.hpp"
#include "../components/Vector2D.hpp"
#include "getActualPosition.hpp"

class CollisionSystem
{
  public:
    void update(EntityManager &entityManager, std::vector<std::pair<int, int>> &playersScores, bool &updateScore);

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
        BONUS,
        UNKNOWN
    };

    EntityType getEntityType(Entity *entity) const;

    // ============================================================
    // MAIN COLLISION HANDLER
    // ============================================================

    void handleCollision(Entity *a, Entity *b, std::vector<std::pair<int, int>> &playersScores, bool &updateScore, EntityManager &entityManager);

    // ============================================================
    // PLAYER COLLISION HANDLERS
    // ============================================================

    void handlePlayerCollision(Entity *player, Entity *other, EntityType otherType);

    void onPlayerHitProjectile(Entity *player, Entity *projectile);

    void onPlayerHitLaser(Entity *player, Entity *laser);
    void onPlayerHitBonus(Entity *player, Entity *bonus);

    // ============================================================
    // ENEMY COLLISION HANDLERS
    // ============================================================

    void handleEnemyCollision(Entity *enemy, Entity *other, EntityType otherType,
                              std::vector<std::pair<int, int>> &playersScores, bool &updateScore, EntityManager &entityManager);

    void onEnemyHitProjectile(Entity *enemy, Entity *projectile, std::vector<std::pair<int, int>> &playersScores,
                              bool &updateScore, EntityManager &entityManager);

    // ============================================================
    // PROJECTILE COLLISION HANDLERS
    // ============================================================

    void handleProjectileCollision(Entity *projectile, Entity *other, EntityType otherType);

    void onProjectileHitLaser(Entity *projectile, Entity *laser);

    // ============================================================
    // LASER COLLISION HANDLERS
    // ============================================================

    void handleLaserCollision(Entity *laser, Entity *other, EntityType otherType);

    // ============================================================
    // JUMPER COLLISION HANDLERS (Flappy Bird)
    // ============================================================

    void handleJumperCollision(Entity *jumper, Entity *other, EntityType otherType);
};