#pragma once
#include "../components/BackgroundScrollComponent.hpp"
#include "../components/SpriteComponent.hpp"
#include "../components/TransformComponent.hpp"
#include "../components/Vector2D.hpp"
#include "../entityManager.hpp"

class BackgroundSystem
{
  public:
    void update(EntityManager &entityManager, float deltaTime)
    {
        auto entities =
            entityManager.getEntitiesWithComponents<TransformComponent, SpriteComponent, BackgroundScrollComponent>();

        for (auto &entity : entities)
        {
            auto &transform = entity->getComponent<TransformComponent>();
            auto &sprite = entity->getComponent<SpriteComponent>();
            auto &background = entity->getComponent<BackgroundScrollComponent>();

            if (!background.active)
                continue;

            transform.position.x += background.scrollSpeed * deltaTime;

            float tileWidth = (float)sprite.width;

            if (transform.position.x <= -tileWidth)
                transform.position.x += 2.0 * tileWidth;
        }
    }
};
