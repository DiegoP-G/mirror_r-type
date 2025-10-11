#pragma once

#include "../entityManager.hpp"

#include "../components/ColliderComponent.hpp"
#include "../components/LaserWarningComponent.hpp"
#include "../components/SpriteComponent.hpp"
#include "../components/TransformComponent.hpp"
#include "../components/Vector2D.hpp"

class LaserWarningSystem
{
  public:
    void update(EntityManager &entityManager, float deltaTime)
    {
        auto entities = entityManager.getEntitiesWithComponents<LaserWarningComponent>();
        for (auto &entity : entities)
        {
            auto &laser = entity->getComponent<LaserWarningComponent>();
            auto &sprite = entity->getComponent<SpriteComponent>();
            if (!entity->hasComponent<TransformComponent>())
            {
                std::cerr << "Entity with LaserWarningComponent missing TransformComponent!" << std::endl;
                continue; // Skip this entity if it doesn't have a TransformComponent
            }
            auto &transform = entity->getComponent<TransformComponent>();
            if (laser.appearanceTime > 0.0f)
            {
                laser.appearanceTime -= deltaTime;
                sprite.isVisible = false;
                continue; // Wait until appearance time is over
            }
            if (!laser.isActive)
            {
                if (!laser.warningShown)
                {
                    laser.warningShown = true;
                    sprite.isVisible = true;
                    sprite.r = 200; // Red color for warning
                    sprite.g = 0;
                    sprite.b = 0;
                    sprite.height = laser.height / 4;
                    transform.position.y += (laser.height - sprite.height) / 2;
                    sprite.width = laser.width;
                }
                laser.warningTime -= deltaTime;
                if (laser.warningTime <= 0.0f)
                {
                    laser.isActive = true;
                    if (entity->hasComponent<ColliderComponent>())
                    {
                        auto &collider = entity->getComponent<ColliderComponent>();
                        collider.isActive = true; // Enable collision when laser is active
                    }
                    sprite.r = 255;
                    transform.position.y -= (laser.height - sprite.height) / 2;
                    sprite.height = laser.height;

                    // Activer le laser (collision, dégâts, sprite différent, etc.)
                }
            }
            else
            {
                // Phase active
                laser.activeTime -= deltaTime;
                if (laser.activeTime <= 0.0f)
                {
                    // Désactiver ou détruire le laser
                    entity->destroy();
                }
            }
        }
    }
};
