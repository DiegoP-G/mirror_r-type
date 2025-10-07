#include "RenderSystem.hpp"

void RenderSystem::update(EntityManager &entityManager)
{
    if (!g_graphics)
        return;

    auto entities = entityManager.getEntitiesWithComponents<TransformComponent, SpriteComponent>();

    for (auto &entity : entities)
    {
        auto &transform = entity->getComponent<TransformComponent>();
        Vector2D position = getActualPosition(entity);

        auto &sprite = entity->getComponent<SpriteComponent>();

        if (!sprite.isVisible)
            continue;

        sf::Texture *texture = g_graphics->getTexture(sprite.spriteTexture);

        if (texture)
        {
            sf::Sprite sfmlSprite(*texture);
            sfmlSprite.setPosition(position.x, position.y);
            sfmlSprite.setScale(sprite.width / (float)texture->getSize().x,
                                sprite.height / (float)texture->getSize().y);

            g_graphics->getWindow().draw(sfmlSprite);
        }
        else
        {
            g_graphics->drawRect(position.x, position.y, sprite.width, sprite.height, sprite.r, sprite.g, sprite.b,
                                 sprite.a);
        }
    }

    auto animatedEntities = entityManager.getEntitiesWithComponents<TransformComponent, AnimatedSpriteComponent>();

    for (auto &entity : animatedEntities)
    {
        auto &transform = entity->getComponent<TransformComponent>();
        Vector2D position = getActualPosition(entity);

        if (entity->hasComponent<AnimatedSpriteComponent>())
        {
            auto &animComp = entity->getComponent<AnimatedSpriteComponent>();
            renderAnimatedSprite(animComp, position.x, position.y);
            continue;
        }
    }
}

void RenderSystem::renderAnimatedSprite(AnimatedSpriteComponent &animComp, float x, float y)
{
    sf::Texture *texture = g_graphics->getTexture(animComp.textureID);
    if (!texture)
    {
        std::cerr << "Can't get texture id : " << animComp.textureID << std::endl;
        return;
    }

    sf::Sprite sprite(*texture);
    sprite.setPosition(x, y);
    sprite.setScale(animComp.scale.x, animComp.scale.y);

    sprite.setTextureRect(sf::IntRect(animComp.left, animComp.top, animComp.frameWidth, animComp.frameHeight));
    sprite.setRotation(animComp.rotationAngle);

    g_graphics->getWindow().draw(sprite);
}
