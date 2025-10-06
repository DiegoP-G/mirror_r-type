#pragma once
#include "../GraphicsManager.hpp"
#include "../components/InputComponent.hpp"
#include "../components/Vector2D.hpp"
#include "../components/VelocityComponent.hpp"
#include "../entityManager.hpp"

class InputSystem
{
  public:
    void update(EntityManager &entityManager, float deltaTime);

  private:
    void handleEvents(EntityManager &entityManager);
};