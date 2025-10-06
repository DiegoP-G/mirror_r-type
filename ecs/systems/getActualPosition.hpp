#pragma once
#include "../components/CenteredComponent.hpp"
#include "../components/TransformComponent.hpp"
#include "../components/Vector2D.hpp"
#include "../entityManager.hpp"


inline Vector2D getActualPosition(Entity *entity)
{
    auto &transform = entity->getComponent<TransformComponent>();
    Vector2D position = transform.position;

    // Utiliser hasComponent au lieu d'un try-catch
    if (entity->hasComponent<CenteredComponent>())
    {
        auto &centered = entity->getComponent<CenteredComponent>();
        position.x -= centered.offsetX;
        position.y -= centered.offsetY;
    }
    return position;
}