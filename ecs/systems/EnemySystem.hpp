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
    void update(EntityManager &entityManager, float deltaTime);

  private:
    void enemyFire(EntityManager &entityManager, Entity *enemy);
};