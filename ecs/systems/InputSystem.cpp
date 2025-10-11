#include "InputSystem.hpp"
#include "iostream"

void InputSystem::update(EntityManager &entityManager, float deltaTime)
{
    auto entities = entityManager.getEntitiesWithComponents<InputComponent, VelocityComponent>();

    for (auto &entity : entities)
    {
        auto &input = entity->getComponent<InputComponent>();
        auto &velocity = entity->getComponent<VelocityComponent>();

        velocity.velocity.x = 0.0f;
        velocity.velocity.y = 0.0f;

        const float PLAYER_SPEED = 200.0f;
        if (input.up)
            velocity.velocity.y = -PLAYER_SPEED;
        if (input.down)
            velocity.velocity.y = PLAYER_SPEED;
        if (input.left)
            velocity.velocity.x = -PLAYER_SPEED;
        if (input.right)
        {
            std::cout << "right pressed" << std::endl;
            velocity.velocity.x = PLAYER_SPEED;
        }
        // std::cout << input.enter << std::endl;
        // if (input.enter) {
        //     std::cout << "enter pressed " << std::endl;
        // }
    }
}

void InputSystem::handleEvents(EntityManager &entityManager)
{
    if (!g_graphics)
        return;

    std::cout << "test" << std::endl;
    sf::Event event;
    while (g_graphics->getWindow().pollEvent(event))
    {
        if (event.type == sf::Event::Closed)
        {
            // Handle window close
        }

        if (event.type == sf::Event::KeyPressed || event.type == sf::Event::KeyReleased)
        {
            bool isPressed = (event.type == sf::Event::KeyPressed);

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
                    std::cout << "iright pressed" << std::endl;
                    input.right = isPressed;
                    break;
                case sf::Keyboard::Space:
                    input.fire = isPressed;
                    break;
                case sf::Keyboard::Enter:
                    input.enter = isPressed;
                    break;
                default:
                    break;
                }
            }
        }
    }
}
