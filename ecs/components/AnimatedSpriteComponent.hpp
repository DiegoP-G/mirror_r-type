#pragma once
#include "../IComponent.hpp"
#include "Vector2D.hpp"

class AnimatedSpriteComponent : public IComponent
{
  public:
    int textureID;
    int currentFrame;
    int left;
    int top;
    int frameWidth;
    int frameHeight;
    int totalFrames;
    float animationInterval;
    float rotationAngle;
    bool hideAfterOneCycle;

    Vector2D scale;
    float elapsedTime;

    enum Direction
    {
        Default,
        Up,
        Down
    } currentDirection;

    AnimatedSpriteComponent(int textureID, int left, int top, int frameWidth, int frameHeight,
      int totalFrames, float interval, float rotation = 0.0f, Vector2D scale = {1.0f, 1.0f},
      int currentFrame = 0, bool hideAfterOneCycle = false);

    void setFrame(int frame);
    void update(float deltaTime) override;
    void init() override;
    std::vector<uint8_t> serialize() const override;
    static AnimatedSpriteComponent deserialize(const uint8_t *data, size_t size);

    void setElapsedTime(float t) { elapsedTime = t; };
};