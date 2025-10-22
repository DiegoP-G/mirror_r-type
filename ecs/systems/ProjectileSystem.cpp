#include "ProjectileSystem.hpp"

void ProjectileSystem::update(EntityManager &entityManager, float deltaTime)
{
    std::vector<EntityID> toDestroy;

    auto entities = entityManager.getEntitiesWithComponents<ProjectileComponent>();
    for (auto &entity : entities)
    {
        auto &projectile = entity->getComponent<ProjectileComponent>();
        projectile.remainingLife -= deltaTime;
        if (projectile.remainingLife <= 0)
        {
            toDestroy.push_back(entity->getID());
        }
    }

    auto inactiveEntities = entityManager.getInactiveEntitiesWithComponents<ProjectileComponent>();
    for (auto &entity : inactiveEntities)
    {
        if (!entity->isActive())
        {
            toDestroy.push_back(entity->getID());
        }
    }

    // Now safely mark for destruction outside of iteration
    for (auto id : toDestroy)
    {
        entityManager.markEntityForDestruction(id);
    }
}
