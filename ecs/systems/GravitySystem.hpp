#pragma once

#include "../components/GravityComponent.hpp"
#include "../components/VelocityComponent.hpp"
#include "../entityManager.hpp"

class GravitySystem
{
  public:
    void update(EntityManager &entityManager, float deltaTime)
    {
        auto entities = entityManager.getEntitiesWithComponents<VelocityComponent, GravityComponent>();

        for (auto &entity : entities)
        {
            auto &velocity = entity->getComponent<VelocityComponent>();
            auto &gravity = entity->getComponent<GravityComponent>();

            // Apply gravity
            velocity.velocity.y += gravity.gravity * deltaTime;

            // Cap velocity at terminal velocity
            if (velocity.velocity.y > gravity.terminalVelocity)
            {
                velocity.velocity.y = gravity.terminalVelocity;
            }
        }
    }
};
