#pragma once

#include "../components/ProjectileComponent.hpp"
#include "../entityManager.hpp"

class ProjectileSystem
{
  public:
    void update(EntityManager &entityManager, float deltaTime);
};