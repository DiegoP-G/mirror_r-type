#pragma once

#include "../components/AnimatedSpriteComponent.hpp"
#include "../components/ColliderComponent.hpp"
#include "../components/PlayerComponent.hpp"
#include "../components/ProjectileComponent.hpp"
#include "../components/SpriteComponent.hpp"
#include "../components/TransformComponent.hpp"
#include "../components/VelocityComponent.hpp"
#include "../entityManager.hpp"

class PlayerSystem
{
  public:
    void update(EntityManager &entityManager, float deltaTime);

  private:
    void fire(EntityManager &entityManager, Entity *player);
    void handlePositionPalyer(Entity *&entity);
};