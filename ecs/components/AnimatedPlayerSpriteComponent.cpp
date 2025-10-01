#include "AnimatedPlayerSpriteComponent.hpp"
#include <cstring>

AnimatedPlayerSpriteComponent::AnimatedPlayerSpriteComponent(const sf::Texture &tex, int frameWidth, int frameHeight,
                                                             float interval, Vector2D scale)
    : texture(&tex), currentFrame(2), frameWidth(frameWidth), frameHeight(frameHeight), animationInterval(interval),
      scale(scale), currentDirection(Default)
{
    sprite.setTexture(*texture);
    sprite.setScale(scale.x, scale.y);
    setFrame(currentFrame);
}

void AnimatedPlayerSpriteComponent::setFrame(int frame)
{
    sprite.setTextureRect(sf::IntRect(frameWidth * frame, 0, frameWidth, frameHeight));
}

void AnimatedPlayerSpriteComponent::updateAnimation(Direction newDirection)
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

void AnimatedPlayerSpriteComponent::update(float deltaTime)
{
}

void AnimatedPlayerSpriteComponent::init()
{
}

std::vector<uint8_t> AnimatedPlayerSpriteComponent::serialize() const
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

    return data;
}

AnimatedPlayerSpriteComponent AnimatedPlayerSpriteComponent::deserialize(const uint8_t *data)
{
    static sf::Texture defaultTexture;
    AnimatedPlayerSpriteComponent comp(defaultTexture, 32, 32, 0.1f);

    size_t offset = 0;
    std::memcpy(&comp.currentFrame, data + offset, sizeof(int));
    offset += sizeof(int) * 3; // Skip frameWidth and frameHeight (const)
    std::memcpy(&comp.animationInterval, data + offset, sizeof(float));
    offset += sizeof(float);
    comp.scale = Vector2D::deserialize(data + offset);
    offset += sizeof(Vector2D);
    std::memcpy(&comp.currentDirection, data + offset, sizeof(Direction));

    return comp;
}
