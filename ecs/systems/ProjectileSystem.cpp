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
    }
    auto inactiveEntities = entityManager.getInactiveEntitiesWithComponents<ProjectileComponent>();
    for (auto &entity : inactiveEntities)
    {

        if (entity->isActive() == false)
        {
            std::cout << " YES DESTROY HIT" << std::endl;
            entityManager.markEntityForDestruction(entity->getID());
        }
    }
}
