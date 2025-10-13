#pragma once
#include "../components/TransformComponent.hpp"
#include "../components/VelocityComponent.hpp"
#include "../entityManager.hpp"
#include <cstdlib>

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
            if (velocity.sineMovement)
            {
                velocity.time += deltaTime;
                transform.position.x += velocity.velocity.x * deltaTime;
                transform.position.y += velocity.velocity.y * deltaTime + sinf(velocity.time * 4);
            }
            else if (velocity.reverseSineMovement)
            {
                velocity.time += deltaTime;
                transform.position.x += velocity.velocity.x * deltaTime;
                transform.position.y += velocity.velocity.y * deltaTime - sinf(velocity.time * 4);
            }
            else
            {
                transform.position += velocity.velocity * deltaTime;
            }
        }
    }
};