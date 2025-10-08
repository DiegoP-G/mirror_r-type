#pragma once

#include "../components/BonusComponent.hpp"
#include "../entityManager.hpp"

class BonusSystem
{
  public:
    void update(EntityManager &entityManager, float deltaTime);
};