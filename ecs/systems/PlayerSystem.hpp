#pragma once

#include "../components/AnimatedSpriteComponent.hpp"
#include "../components/ColliderComponent.hpp"
#include "../components/InputComponent.hpp"
#include "../components/PlayerComponent.hpp"
#include "../components/ProjectileComponent.hpp"
#include "../components/SpriteComponent.hpp"
#include "../components/TransformComponent.hpp"
#include "../components/VelocityComponent.hpp"
#include "../entityManager.hpp"

constexpr static float WARP_DISTANCE = 150.0f;
constexpr static float WARP_COOLDOWN = 2.0f;

constexpr static float FIRE_STAMINA_COST = 10.0f;
constexpr static float WARP_STAMINA_COST = 100.0f;

constexpr static float NORMAL_SPEED = 10.0f;
constexpr static float WARP_SPEED_MULTIPLIER = 20.0f;
class PlayerSystem
{
  public:
    void update(EntityManager &entityManager, float deltaTime, bool client = false);

  private:
    void fire(EntityManager &entityManager, Entity *player);
    void handlePositionPlayer(Entity *&entity);
    void handleWarp(Entity *&entity, float deltaTime);
    void queueBulletCreation(Entity *owner);
};
