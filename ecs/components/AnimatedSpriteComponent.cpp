#include "AnimatedSpriteComponent.hpp"
#include "../GraphicsManager.hpp"
#include <cstring>

AnimatedSpriteComponent::AnimatedSpriteComponent(int tex, int frameWidth, int frameHeight, float interval,
                                                 Vector2D scale)
    : texture(tex), currentFrame(2), frameWidth(frameWidth), frameHeight(frameHeight), animationInterval(interval),
      scale(scale), currentDirection(Default)
{
    sprite.setTexture(*g_graphics->getTexture(texture));
    sprite.setScale(scale.x, scale.y);
    setFrame(currentFrame);
}

void AnimatedSpriteComponent::setFrame(int frame)
{
    sprite.setTextureRect(sf::IntRect(frameWidth * frame, 0, frameWidth, frameHeight));
}

void AnimatedSpriteComponent::updateAnimation(Direction newDirection)
{
    if (newDirection != currentDirection)
    {
        currentDirection = newDirection;
    }

    if (animationClock.getElapsedTime().asSeconds() >= animationInterval)
    {
        if (currentDirection == Up && currentFrame < 4)
        {
            currentFrame++;
        }
        else if (currentDirection == Down && currentFrame > 0)
        {
            currentFrame--;
        }
        else if (currentDirection == Default)
        {
            if (currentFrame > 2)
                currentFrame--;
            else if (currentFrame < 2)
                currentFrame++;
        }
        setFrame(currentFrame);
        animationClock.restart();
    }
}

void AnimatedSpriteComponent::update(float deltaTime)
{
}

void AnimatedSpriteComponent::init()
{
}

std::vector<uint8_t> AnimatedSpriteComponent::serialize() const
{
    std::vector<uint8_t> data;

    data.insert(data.end(), reinterpret_cast<const uint8_t *>(&currentFrame),
                reinterpret_cast<const uint8_t *>(&currentFrame) + sizeof(int));
    data.insert(data.end(), reinterpret_cast<const uint8_t *>(&frameWidth),
                reinterpret_cast<const uint8_t *>(&frameWidth) + sizeof(int));
    data.insert(data.end(), reinterpret_cast<const uint8_t *>(&frameHeight),
                reinterpret_cast<const uint8_t *>(&frameHeight) + sizeof(int));
    data.insert(data.end(), reinterpret_cast<const uint8_t *>(&animationInterval),
                reinterpret_cast<const uint8_t *>(&animationInterval) + sizeof(float));

    auto scaleData = scale.serialize();
    data.insert(data.end(), scaleData.begin(), scaleData.end());

    data.insert(data.end(), reinterpret_cast<const uint8_t *>(&currentDirection),
                reinterpret_cast<const uint8_t *>(&currentDirection) + sizeof(Direction));
    data.insert(data.end(), reinterpret_cast<const uint8_t *>(&texture),
                reinterpret_cast<const uint8_t *>(&texture) + sizeof(int));
    return data;
}

AnimatedSpriteComponent AnimatedSpriteComponent::deserialize(const uint8_t *data, size_t size)
{
    // currentFrame (int) + animationInterval (float) + scale (Vector2D) + currentDirection (Direction) + texture (int)
    size_t expectedSize = sizeof(int) + sizeof(float) + sizeof(Vector2D) + sizeof(Direction) + sizeof(int);
    if (size < expectedSize)
    {
        throw("AnimatedSpriteComponent::deserialize - données trop petites");
    }

    AnimatedSpriteComponent comp(0, 32, 32, 0.1f);
    size_t offset = 0;

    std::memcpy(&comp.currentFrame, data + offset, sizeof(int));
    offset += sizeof(int);

    // Skip frameWidth and frameHeight (const) => rien à faire car valeurs par défaut

    std::memcpy(&comp.animationInterval, data + offset, sizeof(float));
    offset += sizeof(float);

    comp.scale = Vector2D::deserialize(data + offset, sizeof(Vector2D));
    offset += sizeof(Vector2D);

    std::memcpy(&comp.currentDirection, data + offset, sizeof(Direction));
    offset += sizeof(Direction);

    std::memcpy(&comp.texture, data + offset, sizeof(int));

    // Appliquer le rendu
    comp.sprite.setTexture(*g_graphics->getTexture(comp.texture));
    comp.sprite.setScale(comp.scale.x, comp.scale.y);

    return comp;
}
