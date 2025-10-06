#pragma once

#include "../components/AnimatedSpriteComponent.hpp"
#include "../components/ColliderComponent.hpp"
#include "../components/PlayerComponent.hpp"
#include "../components/ProjectileComponent.hpp"
#include "../components/SpriteComponent.hpp"
#include "../components/TransformComponent.hpp"
#include "../components/VelocityComponent.hpp"
#include "../entityManager.hpp"

class PlayerSystem
{
  public:
    void update(EntityManager &entityManager, float deltaTime)
    {
        auto entities = entityManager.getEntitiesWithComponents<InputComponent, PlayerComponent>();

        for (auto &entity : entities)
        {
            auto &input = entity->getComponent<InputComponent>();

            // Fire a bullet when space is pressed
            if (input.fire)
            {
                fire(entityManager, entity);
                input.fire = false; // Reset fire input
            }

            if (!entity->hasComponent<AnimatedSpriteComponent>())
                continue;
            auto &animatedSprite = entity->getComponent<AnimatedSpriteComponent>();
            // Determine direction based on input
            AnimatedSpriteComponent::Direction direction = AnimatedSpriteComponent::Default;
            if (input.up)
            {
                direction = AnimatedSpriteComponent::Up;
            }
            else if (input.down)
            {
                direction = AnimatedSpriteComponent::Down;
            }

            // Update animation based on direction
            animatedSprite.updateAnimation(direction);
        }
    }

  private:
    void fire(EntityManager &entityManager, Entity *player)
    {
        auto &transform = player->getComponent<TransformComponent>();

        // Create bullet entity
        auto &bullet = entityManager.createEntity();

        // Position the bullet at the player's position
        bullet.addComponent<TransformComponent>(transform.position.x + 32.0f, // Offset to fire from the front
                                                transform.position.y + 16.0f  // Center height
        );

        // Add bullet components
        bullet.addComponent<VelocityComponent>(300.0f, 0.0f);
        bullet.addComponent<SpriteComponent>(8, 8, 255, 0, 0);                  // Red bullet
        bullet.addComponent<ColliderComponent>(8.0f, 8.0f);                     // Small hitbox
        bullet.addComponent<ProjectileComponent>(10.0f, 2.0f, player->getID()); // Damage, lifetime, owner
    }
};
