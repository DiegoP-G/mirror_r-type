#include "AnimationSystem.hpp"
#include "../components/AnimatedSpriteComponent.hpp"
#include "../components/InputComponent.hpp"
#include "../components/PlayerComponent.hpp"
#include "../components/TransformComponent.hpp"

#include "../entity.hpp"
void AnimationSystem::update(EntityManager &entityManager, float deltaTime)
{
    auto entities = entityManager.getEntitiesWithComponents<InputComponent, PlayerComponent>();

    for (auto &entity : entities)
    {
        auto &input = entity->getComponent<InputComponent>();
        handleAnimation(entity, input, deltaTime);
    }

    auto sprites = entityManager.getEntitiesWithComponents<AnimatedSpriteComponent, TransformComponent>();

    for (auto &entity : sprites)
    {
        if (entity->hasComponent<PlayerComponent>())
        {
            continue;
        }

        auto &animatedSprite = entity->getComponent<AnimatedSpriteComponent>();

        if (animatedSprite.totalFrames == 1)
        {
            continue;
        }

        animatedSprite.elapsedTime += deltaTime;
        if (animatedSprite.elapsedTime >= animatedSprite.animationInterval)
        {
            if (animatedSprite.hideAfterOneCycle && animatedSprite.currentFrame == animatedSprite.totalFrames - 1)
            {
                entityManager.destroyEntityByID(entity->getID());
                continue;
            }
            else
            {
                animatedSprite.currentFrame = (animatedSprite.currentFrame + 1) % animatedSprite.totalFrames;
                animatedSprite.setFrame(animatedSprite.currentFrame);
                animatedSprite.elapsedTime = 0.0f;
            }
        }
    }
}

void AnimationSystem::handleAnimation(Entity *&entity, InputComponent &input, float deltaTime)
{
    if (!entity->hasComponent<AnimatedSpriteComponent>())
    {
        std::cout << "Entity " << entity->getID() << " does not have AnimatedSpriteComponent!" << std::endl;
        return;
    }

    auto &animatedSprite = entity->getComponent<AnimatedSpriteComponent>();

    AnimatedSpriteComponent::Direction direction = AnimatedSpriteComponent::Default;

    // std::cout << "Input state: up=" << input.up << ", down=" << input.down << ", left=" << input.left << ", right="
    // << input.right << ", fire=" << input.fire << std::endl;

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

    // std::cout << "Current Frame: " << animatedSprite.currentFrame << ", Direction: " <<
    // animatedSprite.currentDirection << std::endl;
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
        animatedSprite.setFrame(animatedSprite.currentFrame);
        animatedSprite.elapsedTime = 0.0f;
    }
}