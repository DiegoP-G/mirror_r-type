#include "BonusSystem.hpp"

void BonusSystem::update(EntityManager &entityManager, float deltaTime)
{
    auto entities = entityManager.getEntitiesWithComponents<BonusComponent>();

    auto inactiveEntities = entityManager.getInactiveEntitiesWithComponents<BonusComponent>();

    for (auto &entity : inactiveEntities)
    {
        if (entity->isActive() == false)
        {
            entityManager.markEntityForDestruction(entity->getID());
        }
    }
}
