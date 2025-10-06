#include "AnimatedSpriteComponent.hpp"
#include <cstring>

AnimatedSpriteComponent::AnimatedSpriteComponent(int texture, int frameWidth, int frameHeight, float interval,
                                                 Vector2D scale)
    : textureID(texture), frameWidth(frameWidth), frameHeight(frameHeight), animationInterval(interval), scale(scale),
      currentFrame(2), currentDirection(Default), elapsedTime(0.0f)
{
}

void AnimatedSpriteComponent::setFrame(int frame)
{
    currentFrame = frame;
}

void AnimatedSpriteComponent::updateAnimation(Direction newDirection, float deltaTime)
{
    if (newDirection != currentDirection)
    {
        currentDirection = newDirection;
    }

    elapsedTime += deltaTime;

    if (elapsedTime >= animationInterval)
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
        elapsedTime = 0.0f;
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
    
    data.insert(data.end(), reinterpret_cast<const uint8_t*>(&textureID),
                reinterpret_cast<const uint8_t*>(&textureID) + sizeof(int));
    data.insert(data.end(), reinterpret_cast<const uint8_t*>(&currentFrame),
                reinterpret_cast<const uint8_t*>(&currentFrame) + sizeof(int));
    data.insert(data.end(), reinterpret_cast<const uint8_t*>(&frameWidth),
                reinterpret_cast<const uint8_t*>(&frameWidth) + sizeof(int));
    data.insert(data.end(), reinterpret_cast<const uint8_t*>(&frameHeight),
                reinterpret_cast<const uint8_t*>(&frameHeight) + sizeof(int));
    data.insert(data.end(), reinterpret_cast<const uint8_t*>(&animationInterval),
                reinterpret_cast<const uint8_t*>(&animationInterval) + sizeof(float));
    
    auto scaleData = scale.serialize();
    data.insert(data.end(), scaleData.begin(), scaleData.end());
    
    data.insert(data.end(), reinterpret_cast<const uint8_t*>(&elapsedTime),
                reinterpret_cast<const uint8_t*>(&elapsedTime) + sizeof(float));
    data.push_back(static_cast<uint8_t>(currentDirection));
    
    return data;
}

AnimatedSpriteComponent AnimatedSpriteComponent::deserialize(const uint8_t *data, size_t size)
{
    size_t offset = 0;
    
    int textureID, currentFrame, frameWidth, frameHeight;
    float animationInterval, elapsedTime;
    Vector2D scale;
    Direction dir;
    
    std::memcpy(&textureID, data + offset, sizeof(int));
    offset += sizeof(int);
    std::memcpy(&currentFrame, data + offset, sizeof(int));
    offset += sizeof(int);
    std::memcpy(&frameWidth, data + offset, sizeof(int));
    offset += sizeof(int);
    std::memcpy(&frameHeight, data + offset, sizeof(int));
    offset += sizeof(int);
    std::memcpy(&animationInterval, data + offset, sizeof(float));
    offset += sizeof(float);
    
    scale = Vector2D::deserialize(data + offset, sizeof(Vector2D));
    offset += sizeof(Vector2D);
    
    std::memcpy(&elapsedTime, data + offset, sizeof(float));
    offset += sizeof(float);
    dir = static_cast<Direction>(data[offset]);
    
    AnimatedSpriteComponent comp(textureID, frameWidth, frameHeight, animationInterval, scale);
    comp.currentFrame = currentFrame;
    comp.currentDirection = dir;
    comp.elapsedTime = elapsedTime;
    
    return comp;
}