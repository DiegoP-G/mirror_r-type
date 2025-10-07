#include "PlayerSystem.hpp"

void PlayerSystem::update(EntityManager &entityManager, float deltaTime)
{
    auto entities = entityManager.getEntitiesWithComponents<InputComponent, PlayerComponent>();

    for (auto &entity : entities)
    {
        auto &input = entity->getComponent<InputComponent>();

        if (input.fire)
        {
            fire(entityManager, entity);
            input.fire = false;
        }

        handleAnimation(entity, input, deltaTime);
        handlePositionPalyer(entity);
    }
}

void PlayerSystem::handlePositionPalyer(Entity *&entity)
{
    if (!entity->hasComponent<TransformComponent>())
        return;
    auto &tranform = entity->getComponent<TransformComponent>();
    if (tranform.position.x > 800 - 32)
        tranform.position.x = 800 - 32;

    if (tranform.position.x < 0)
        tranform.position.x = 0;

    if (tranform.position.y < 0)
        tranform.position.y = 0;

    if (tranform.position.y > 600 - 32)
        tranform.position.y = 600 - 32;
}

void PlayerSystem::handleAnimation(Entity *&entity, InputComponent &input, float deltaTime)
{
    if (!entity->hasComponent<AnimatedSpriteComponent>())
        return;
    auto &animatedSprite = entity->getComponent<AnimatedSpriteComponent>();

    AnimatedSpriteComponent::Direction direction = AnimatedSpriteComponent::Default;
    if (input.up)
    {
        direction = AnimatedSpriteComponent::Up;
    }
    else if (input.down)
    {
        direction = AnimatedSpriteComponent::Down;
    }

    if (direction != animatedSprite.currentDirection)
    {
        animatedSprite.currentDirection = direction;
    }

    animatedSprite.elapsedTime += deltaTime;

    if (animatedSprite.elapsedTime >= animatedSprite.animationInterval)
    {
        if (animatedSprite.currentDirection == AnimatedSpriteComponent::Up && animatedSprite.currentFrame < 4)
        {
            animatedSprite.currentFrame++;
        }
        else if (animatedSprite.currentDirection == AnimatedSpriteComponent::Down && animatedSprite.currentFrame > 0)
        {
            animatedSprite.currentFrame--;
        }
        else if (animatedSprite.currentDirection == AnimatedSpriteComponent::Default)
        {
            if (animatedSprite.currentFrame > 2)
                animatedSprite.currentFrame--;
            else if (animatedSprite.currentFrame < 2)
                animatedSprite.currentFrame++;
        }
        animatedSprite.elapsedTime = 0.0f;
    }
}

void PlayerSystem::fire(EntityManager &entityManager, Entity *player)
{
    auto &transform = player->getComponent<TransformComponent>();

    auto &bullet = entityManager.createEntity();

    bullet.addComponent<TransformComponent>(transform.position.x + 32.0f, transform.position.y + 16.0f);

    bullet.addComponent<VelocityComponent>(300.0f, 0.0f);
    bullet.addComponent<SpriteComponent>(8, 8, 255, 0, 0);
    bullet.addComponent<ColliderComponent>(8.0f, 8.0f);
    bullet.addComponent<ProjectileComponent>(10.0f, 2.0f, player->getID());
}
