#include "ProjectileSystem.hpp"

void ProjectileSystem::update(EntityManager &entityManager, float deltaTime)
{
    auto entities = entityManager.getEntitiesWithComponents<ProjectileComponent>();

    for (auto &entity : entities)
    {
        auto &projectile = entity->getComponent<ProjectileComponent>();

        projectile.remainingLife -= deltaTime;
        if (projectile.remainingLife <= 0)
        {
            entityManager.markEntityForDestruction(entity->getID());
        }
        if (!entity->isActive())
            entityManager.markEntityForDestruction(entity->getID());
    }
}
