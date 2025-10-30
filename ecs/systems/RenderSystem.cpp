#include "RenderSystem.hpp"
#include "../components/PlayerComponent.hpp"
#include "../components/TextComponent.hpp"

void RenderSystem::draw(Entity *entity)
{
    auto &transform = entity->getComponent<TransformComponent>();
    Vector2D position = getActualPosition(entity);

    if (entity->hasComponent<TextComponent>())
    {
        auto &textComp = entity->getComponent<TextComponent>();

        g_graphics->drawText(textComp.text, position.x, position.y - 30, 255, 255, 255, true, 15);
    }

    if (entity->hasComponent<AnimatedSpriteComponent>())
    {
        auto &animComp = entity->getComponent<AnimatedSpriteComponent>();
        renderAnimatedSprite(animComp, position.x, position.y);
        return;
    }

    auto &sprite = entity->getComponent<SpriteComponent>();

    if (!sprite.isVisible)
        return;

    sf::Texture *texture = g_graphics->getTexture(sprite.spriteTexture);

    if (texture)
    {
        sf::Sprite sfmlSprite(*texture);
        sfmlSprite.setPosition(position.x, position.y);
        sfmlSprite.setScale(sprite.width / (float)texture->getSize().x, sprite.height / (float)texture->getSize().y);

        g_graphics->getWindow().draw(sfmlSprite);
    }
    else
    {
        g_graphics->drawRect(position.x, position.y, sprite.width, sprite.height, sprite.r, sprite.g, sprite.b,
                             sprite.a);
    }
}

void RenderSystem::drawStaminaBar(Entity *entity)
{
    if (!entity->hasComponent<PlayerComponent>() || !entity->hasComponent<TransformComponent>())
        return;

    auto &playerComp = entity->getComponent<PlayerComponent>();
    Vector2D position = getActualPosition(entity);

    float ratio = std::clamp(playerComp.stamina / playerComp.maxStamina, 0.0f, 1.0f);
    float barWidth = 50.0f;
    float barHeight = 4.0f;
    float offsetY = 27.0f;
    float offsetX = -25.0f;

    g_graphics->drawRect(position.x + offsetX, position.y + offsetY, barWidth, barHeight, 0, 100, 100, 200);
    g_graphics->drawRect(position.x + offsetX, position.y + offsetY, barWidth * ratio, barHeight, 0, 255, 255, 255);
}

void RenderSystem::update(EntityManager &entityManager)
{
    if (!g_graphics)
        return;

    // Draw non-health entities
    auto entities = entityManager.getEntitiesWithComponents<TransformComponent, SpriteComponent>();
    for (auto &entity : entities)
    {
        if (!entity->hasComponent<HealthBarComponent>())
            draw(entity);

        if (entity->hasComponent<PlayerComponent>())
            drawStaminaBar(entity); // ! draw stamina above player
    }

    // Draw animated entities
    auto animatedEntities = entityManager.getEntitiesWithComponents<TransformComponent, AnimatedSpriteComponent>();
    for (auto &entity : animatedEntities)
    {
        if (!entity->hasComponent<ShieldComponent>())
            draw(entity);
    }

    // Draw health bars
    auto healthBarEntities = entityManager.getEntitiesWithComponents<TransformComponent, HealthBarComponent>();
    for (auto &entity : healthBarEntities)
        drawHealthBar(entity);

    // Draw shields
    auto shieldEntities = entityManager.getEntitiesWithComponents<TransformComponent, ShieldComponent>();
    for (auto &entity : shieldEntities)
    {
        auto &shieldComp = entity->getComponent<ShieldComponent>();

        if (shieldComp.shieldLeft <= 0)
            continue;
        std::cout << "FOUND ACTIVE SHIELD, TRY TO DRAW" << std::endl;
        for (auto player : entityManager.getEntitiesWithComponent<PlayerComponent>())
        {
            if (player->getComponent<PlayerComponent>().playerID == shieldComp.ownerID)
            {
                auto &transformComp = player->getComponent<TransformComponent>();
                renderAnimatedSprite(entity->getComponent<AnimatedSpriteComponent>(), transformComp.position.x + 0.5f,
                                     transformComp.position.y);
                break;
            }
        }
    }
}

void RenderSystem::drawHealthBar(Entity *entity)
{
    auto &healthComp = entity->getComponent<HealthComponent>();
    auto &healthBarComp = entity->getComponent<HealthBarComponent>();
    Vector2D position = getActualPosition(entity);

    float healthPercentage = healthComp.health / healthComp.maxHealth;

    float barWidth = healthBarComp.width * healthPercentage;
    float barHeight = healthBarComp.height;
    float offsetY = healthBarComp.offsetY;
    float offsetX = healthBarComp.offsetX;

    if (entity->hasComponent<PlayerComponent>())
    {
        g_graphics->drawRect(position.x + offsetX, position.y + offsetY, healthBarComp.width, barHeight, 255, 0, 0,
                             255);
        g_graphics->drawRect(position.x + offsetX, position.y + offsetY, barWidth, barHeight, 0, 255, 0, 255);
    }
    else
    {
        g_graphics->drawRect(position.x + offsetX, position.y + offsetY, healthBarComp.width, barHeight, 255, 0, 0, 255);
        g_graphics->drawRect(position.x + offsetX, position.y + offsetY, barWidth, barHeight, 0, 255, 0, 255);
    }
    draw(entity);
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
    sprite.setOrigin(animComp.frameWidth / 2.0f, animComp.frameHeight / 2.0f);
    sprite.setRotation(animComp.rotationAngle);

    g_graphics->getWindow().draw(sprite);
}
