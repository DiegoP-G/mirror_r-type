#include "AnimatedSpriteComponent.hpp"
#include <cstring>
#include <iostream>

AnimatedSpriteComponent::AnimatedSpriteComponent(int textID, int left, int top, int frameWidth, int frameHeight,
                                                 int totalFrames, float interval, float rotation, Vector2D scale,
                                                 int currentFrame, bool hideAfter)
    : textureID(textID), left(left), top(top), frameWidth(frameWidth), frameHeight(frameHeight),
      totalFrames(totalFrames), animationInterval(interval), rotationAngle(rotation), scale(scale),
      currentFrame(currentFrame), currentDirection(Default), elapsedTime(0.0f), hideAfterOneCycle(hideAfter)
{
}

void AnimatedSpriteComponent::setFrame(int frame)
{
    left = frameWidth * frame;
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

    data.insert(data.end(), reinterpret_cast<const uint8_t *>(&textureID),
                reinterpret_cast<const uint8_t *>(&textureID) + sizeof(int));
    data.insert(data.end(), reinterpret_cast<const uint8_t *>(&currentFrame),
                reinterpret_cast<const uint8_t *>(&currentFrame) + sizeof(int));
    data.insert(data.end(), reinterpret_cast<const uint8_t *>(&left),
                reinterpret_cast<const uint8_t *>(&left) + sizeof(int));
    data.insert(data.end(), reinterpret_cast<const uint8_t *>(&top),
                reinterpret_cast<const uint8_t *>(&top) + sizeof(int));
    data.insert(data.end(), reinterpret_cast<const uint8_t *>(&frameWidth),
                reinterpret_cast<const uint8_t *>(&frameWidth) + sizeof(int));
    data.insert(data.end(), reinterpret_cast<const uint8_t *>(&frameHeight),
                reinterpret_cast<const uint8_t *>(&frameHeight) + sizeof(int));
    data.insert(data.end(), reinterpret_cast<const uint8_t *>(&totalFrames),
                reinterpret_cast<const uint8_t *>(&totalFrames) + sizeof(int));
    data.insert(data.end(), reinterpret_cast<const uint8_t *>(&animationInterval),
                reinterpret_cast<const uint8_t *>(&animationInterval) + sizeof(float));
    data.insert(data.end(), reinterpret_cast<const uint8_t *>(&rotationAngle),
                reinterpret_cast<const uint8_t *>(&rotationAngle) + sizeof(float));

    auto scaleData = scale.serialize();
    data.insert(data.end(), scaleData.begin(), scaleData.end());

    data.insert(data.end(), reinterpret_cast<const uint8_t *>(&elapsedTime),
                reinterpret_cast<const uint8_t *>(&elapsedTime) + sizeof(float));
    data.push_back(static_cast<uint8_t>(currentDirection));

    // Serialize hideAfterOneCycle as a single byte (1 for true, 0 for false)
    data.push_back(hideAfterOneCycle ? 1 : 0);

    return data;
}

AnimatedSpriteComponent AnimatedSpriteComponent::deserialize(const uint8_t *data, size_t size)
{
    size_t offset = 0;

    int textureID, currentFrame, left, top, frameWidth, frameHeight, totalFrames;
    float animationInterval, rotationAngle, elapsedTime;
    Vector2D scale;
    Direction dir;

    std::memcpy(&textureID, data + offset, sizeof(int));
    offset += sizeof(int);
    std::memcpy(&currentFrame, data + offset, sizeof(int));
    offset += sizeof(int);
    std::memcpy(&left, data + offset, sizeof(int));
    offset += sizeof(int);
    std::memcpy(&top, data + offset, sizeof(int));
    offset += sizeof(int);
    std::memcpy(&frameWidth, data + offset, sizeof(int));
    offset += sizeof(int);
    std::memcpy(&frameHeight, data + offset, sizeof(int));
    offset += sizeof(int);
    std::memcpy(&totalFrames, data + offset, sizeof(int));
    offset += sizeof(int);
    std::memcpy(&animationInterval, data + offset, sizeof(float));
    offset += sizeof(float);
    std::memcpy(&rotationAngle, data + offset, sizeof(float));
    offset += sizeof(float);

    scale = Vector2D::deserialize(data + offset, sizeof(Vector2D));
    offset += sizeof(Vector2D);

    std::memcpy(&elapsedTime, data + offset, sizeof(float));
    offset += sizeof(float);
    dir = static_cast<Direction>(data[offset]);
    offset += sizeof(uint8_t);

    // Deserialize hideAfterOneCycle (1 for true, 0 for false)
    bool hideAfter = data[offset] != 0;

    AnimatedSpriteComponent comp(textureID, left, top, frameWidth, frameHeight, totalFrames, animationInterval,
                                 rotationAngle, scale, currentFrame, hideAfter);
    comp.currentDirection = dir;
    comp.elapsedTime = elapsedTime;

    return comp;
}