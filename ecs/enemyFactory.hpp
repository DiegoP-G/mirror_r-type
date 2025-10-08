#pragma once
#include "GraphicsManager.hpp"
#include "allComponentsInclude.hpp"
#include "components/HealthBarComponent.hpp"
#include "entityManager.hpp"

class EnemyFactory
{
  public:
    static void createEnemy(EntityManager &entityManager, std::string enemyType, const Vector2D &position);
};