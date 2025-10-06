#pragma once

#include "../GraphicsManager.hpp"
#include "../components/AnimatedSpriteComponent.hpp"
#include "../entityManager.hpp"

#include "../components/CenteredComponent.hpp"
#include "../components/SpriteComponent.hpp"
#include "../components/TransformComponent.hpp"
#include "../components/Vector2D.hpp"

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

class RenderSystem
{
  public:
    void update(EntityManager &entityManager)
    {
        if (!g_graphics)
            return;

        auto entities = entityManager.getEntitiesWithComponents<TransformComponent, SpriteComponent>();

        for (auto &entity : entities)
        {
            auto &transform = entity->getComponent<TransformComponent>();
            Vector2D position = getActualPosition(entity);

            if (entity->hasComponent<AnimatedSpriteComponent>())
            {
                auto &animatedSprite = entity->getComponent<AnimatedSpriteComponent>();
                g_graphics->drawAnimatedSprite(animatedSprite, position.x, position.y);
                continue;
            }

            auto &sprite = entity->getComponent<SpriteComponent>();

            if (!sprite.isVisible)
                continue;

            // Si on a un ID de texture valide, l'utiliser
            if (sprite.spriteTexture != -1)
            {
                // CORRECTION : utiliser getTexture(int) au lieu de getTexture(string)
                sf::Texture *texture = g_graphics->getTexture(sprite.spriteTexture);
                if (texture)
                {
                    g_graphics->drawTexture(*texture, position.x, position.y, sprite.width, sprite.height);
                }
                else
                {
                    // Fallback sur un rectangle coloré
                    g_graphics->drawRect(position.x, position.y, sprite.width, sprite.height, sprite.r, sprite.g,
                                         sprite.b, sprite.a);
                }
            }
            else
            {
                // Pas de texture, dessiner un rectangle coloré
                g_graphics->drawRect(position.x, position.y, sprite.width, sprite.height, sprite.r, sprite.g, sprite.b,
                                     sprite.a);
            }
        }
    }
};
