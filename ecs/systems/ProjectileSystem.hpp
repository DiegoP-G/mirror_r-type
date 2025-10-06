#pragma once

#include "../entityManager.hpp"
#include "../components/ProjectileComponent.hpp"

class ProjectileSystem
{
  public:
    void update(EntityManager &entityManager, float deltaTime);
};