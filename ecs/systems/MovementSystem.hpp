#pragma once
#include "../components/TransformComponent.hpp"
#include "../components/VelocityComponent.hpp"
#include "../entityManager.hpp"

class MovementSystem
{
  public:
    void update(EntityManager &entityManager, float deltaTime)
    {
        auto entities = entityManager.getEntitiesWithComponents<TransformComponent, VelocityComponent>();

        for (auto &entity : entities)
        {
            auto &transform = entity->getComponent<TransformComponent>();
            auto &velocity = entity->getComponent<VelocityComponent>();
            transform.position += velocity.velocity * deltaTime;
        }
    }
};