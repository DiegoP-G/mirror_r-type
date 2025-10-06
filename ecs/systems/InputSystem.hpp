#pragma once
#include "../GraphicsManager.hpp"
#include "../components/InputComponent.hpp"
#include "../components/Vector2D.hpp"
#include "../components/VelocityComponent.hpp"
#include "../entityManager.hpp"

class InputSystem
{
  public:
    void update(EntityManager &entityManager, float deltaTime)
    {
        // First, handle SFML events and update InputComponents
        // handleEvents(entityManager);

        // Then update velocities based on input
        auto entities = entityManager.getEntitiesWithComponents<InputComponent, VelocityComponent>();

        for (auto &entity : entities)
        {
            auto &input = entity->getComponent<InputComponent>();
            auto &velocity = entity->getComponent<VelocityComponent>();

            // Reset velocity
            velocity.velocity.x = 0.0f;
            velocity.velocity.y = 0.0f;

            // Update velocity based on input
            const float PLAYER_SPEED = 200.0f;
            if (input.up)
                velocity.velocity.y = -PLAYER_SPEED;
            if (input.down)
                velocity.velocity.y = PLAYER_SPEED;
            if (input.left)
                velocity.velocity.x = -PLAYER_SPEED;
            if (input.right)
                velocity.velocity.x = PLAYER_SPEED;
        }
    }

  private:
    void handleEvents(EntityManager &entityManager)
    {
        if (!g_graphics)
            return;

        sf::Event event;
        while (g_graphics->getWindow().pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
            {
                // You might want to add a way to signal game exit
                // For now, we'll just handle input events
            }

            if (event.type == sf::Event::KeyPressed || event.type == sf::Event::KeyReleased)
            {
                bool isPressed = (event.type == sf::Event::KeyPressed);

                // Update all entities with InputComponent
                auto inputEntities = entityManager.getEntitiesWithComponents<InputComponent>();

                for (auto &entity : inputEntities)
                {
                    auto &input = entity->getComponent<InputComponent>();

                    switch (event.key.code)
                    {
                    case sf::Keyboard::Up:
                        input.up = isPressed;
                        break;
                    case sf::Keyboard::Down:
                        input.down = isPressed;
                        break;
                    case sf::Keyboard::Left:
                        input.left = isPressed;
                        break;
                    case sf::Keyboard::Right:
                        input.right = isPressed;
                        break;
                    case sf::Keyboard::Space:
                        input.fire = isPressed;
                        break;
                    default:
                        break;
                    }
                }
            }
        }
    }
};
