#pragma once
#include "../IComponent.hpp"
#include "Vector2D.hpp"

class AnimatedSpriteComponent : public IComponent
{
  public:
    int textureID;
    int currentFrame;
    int frameWidth;
    int frameHeight;
    float animationInterval;
    Vector2D scale;
    float elapsedTime;

    enum Direction
    {
        Default,
        Up,
        Down
    } currentDirection;

    AnimatedSpriteComponent(int texture, int frameWidth, int frameHeight, float interval,
                            Vector2D scale = {1.0f, 1.0f});

    void setFrame(int frame);
    void updateAnimation(Direction newDirection, float deltaTime);
    void update(float deltaTime) override;
    void init() override;
    std::vector<uint8_t> serialize() const override;
    static AnimatedSpriteComponent deserialize(const uint8_t *data, size_t size);
};