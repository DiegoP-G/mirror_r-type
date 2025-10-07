#include "RenderSystem.hpp"


void RenderSystem::draw(Entity *entity)
{
    auto &transform = entity->getComponent<TransformComponent>();
    Vector2D position = getActualPosition(entity);

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
        sfmlSprite.setScale(
            sprite.width / (float)texture->getSize().x,
            sprite.height / (float)texture->getSize().y
        );
        
        g_graphics->getWindow().draw(sfmlSprite);
    }
    else
    {
        g_graphics->drawRect(position.x, position.y, sprite.width, sprite.height,
                            sprite.r, sprite.g, sprite.b, sprite.a);
    }
}

void RenderSystem::drawHealthBar(Entity *entity)
{
    auto &transform = entity->getComponent<TransformComponent>();
    auto &healthComp = entity->getComponent<HealthComponent>();
    auto &healthBarComp = entity->getComponent<HealthBarComponent>();
    Vector2D position = getActualPosition(entity);

    float healthPercentage = healthComp.health / healthComp.maxHealth;

    std::cout << "Health Percentage: " << healthPercentage << std::endl;

    float barWidth = healthBarComp.width * healthPercentage;
    float barHeight = healthBarComp.height;
    float offsetY = healthBarComp.offsetY;

    g_graphics->drawRect(position.x, position.y + offsetY, healthBarComp.width, barHeight, 255, 0, 0, 255); // Red background
    g_graphics->drawRect(position.x, position.y + offsetY, barWidth, barHeight, 0, 255, 0, 255); // Green health bar
    draw(entity);
}

void RenderSystem::update(EntityManager &entityManager)
{
    if (!g_graphics)
        return;

    auto entities = entityManager.getEntitiesWithComponents<TransformComponent, SpriteComponent>();

    for (auto &entity : entities)
    {
        if (!entity->hasComponent<HealthBarComponent>())
            draw(entity);
    }
    std::vector<Entity *> healthBarEntities = entityManager.getEntitiesWithComponents<TransformComponent, HealthBarComponent>();
    for (auto &entity : healthBarEntities)
    {
        drawHealthBar(entity);
    }
}

void RenderSystem::renderAnimatedSprite(AnimatedSpriteComponent &animComp, float x, float y)
{
    sf::Texture *texture = g_graphics->getTexture(animComp.textureID);
    if (!texture)
        return;

    sf::Sprite sprite(*texture);
    sprite.setPosition(x, y);
    sprite.setScale(animComp.scale.x, animComp.scale.y);
    
    sprite.setTextureRect(sf::IntRect(
        animComp.frameWidth * animComp.currentFrame, 
        0, 
        animComp.frameWidth, 
        animComp.frameHeight
    ));
    
    g_graphics->getWindow().draw(sprite);
}
