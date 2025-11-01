#pragma once

#include "../GraphicsManager.hpp"
#include "../components/CircularMotionComponent.hpp"
#include "../components/ColliderComponent.hpp"
#include "../components/EnemyComponent.hpp"
#include "../components/ProjectileComponent.hpp"
#include "../components/SpriteComponent.hpp"
#include "../components/TransformComponent.hpp"
#include "../components/VelocityComponent.hpp"
#include "../entityManager.hpp"

#include <cmath>

enum SHOOTINGTYPE
{
    STRAIGHT,
    THREE_DISPERSED,
    SINUS,
    ALLDIRECTION,
    CIRCLE,
};

enum MOVEMENTTYPE
{
    ONLY_LEFT,
    SINE,
    STATIC_UP_DOWN
};

struct bulletSpriteType
{
    float left;
    float top;
    float width;
    float height;
};

const bulletSpriteType redBullet{179, 82, 9, 17};
const bulletSpriteType yellowBullet{179, 113, 9, 17};
const bulletSpriteType pinkBullet{179, 144, 9, 17};
const bulletSpriteType whiteBullet{179, 175, 9, 17};

class EnemySystem
{
  public:
    void update(EntityManager &entityManager, float deltaTime);

  private:
    void enemyFire(EntityManager &entityManager, Entity *enemy);
};