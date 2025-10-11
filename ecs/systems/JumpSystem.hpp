#pragma once
#include "../components/InputComponent.hpp"
#include "../components/JumpComponent.hpp"
#include "../components/Vector2D.hpp"
#include "../components/VelocityComponent.hpp"
#include "../entityManager.hpp"

class JumpSystem
{
  public:
    void update(EntityManager &entityManager, float deltaTime)
    {
        auto entities = entityManager.getEntitiesWithComponents<VelocityComponent, InputComponent, JumpComponent>();

        for (auto &entity : entities)
        {
            auto &velocity = entity->getComponent<VelocityComponent>();
            auto &input = entity->getComponent<InputComponent>();
            auto &jump = entity->getComponent<JumpComponent>();

            // Jump when fire is pressed and can jump
            if (input.fire && jump.canJump)
            {
                velocity.velocity.y = jump.jumpStrength;
                input.fire = false; // Reset input
            }
        }
    }
};