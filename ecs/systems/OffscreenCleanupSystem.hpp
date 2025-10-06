#pragma once
#include "../components/BackgroundScrollComponent.hpp"
#include "../components/TransformComponent.hpp"
#include "../entityManager.hpp"

class OffscreenCleanupSystem
{
  public:
    void update(EntityManager &entityManager, float deltaTime)
    {
        auto entities = entityManager.getEntitiesWithComponents<TransformComponent>();

        for (auto &entity : entities)
        {
            auto &transform = entity->getComponent<TransformComponent>();

            if (entity->hasComponent<BackgroundScrollComponent>())
                continue;

            // Generic offscreen cleanup
            if (transform.position.x < -100.0f)
            {
                entity->destroy();
            }
        }
    }
};
