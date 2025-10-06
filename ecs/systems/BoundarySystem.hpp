#pragma once
#include "../components/TransformComponent.hpp"
#include "../components/Vector2D.hpp"
#include "../entityManager.hpp"

class BoundarySystem
{
  public:
    void update(EntityManager &entityManager, float deltaTime)
    {
        auto entities = entityManager.getEntitiesWithComponents<TransformComponent>();

        for (auto &entity : entities)
        {
            auto &transform = entity->getComponent<TransformComponent>();

            // Generic boundary checking - could be configurable
            if (transform.position.y < 0 || transform.position.y > 600 - 32)
            {
                // Mark entity as dead or destroy it
                entity->destroy();
            }
        }
    }
};
