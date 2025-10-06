#pragma once

#include "../entityManager.hpp"

#include "../components/ProjectileComponent.hpp"

#include "../components/ProjectileComponent.hpp"

class ProjectileSystem
{
  public:
    void update(EntityManager &entityManager, float deltaTime)
    {
        auto entities = entityManager.getEntitiesWithComponents<ProjectileComponent>();

        for (auto &entity : entities)
        {
            auto &projectile = entity->getComponent<ProjectileComponent>();

            // Update lifetime
            projectile.remainingLife -= deltaTime;
            if (projectile.remainingLife <= 0)
            {
                entity->destroy();
            }
        }
    }
};
