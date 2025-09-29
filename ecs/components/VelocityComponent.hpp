#pragma once
#include "../components.hpp"
#include "Vector2D.hpp"

class VelocityComponent : public Component
{
  public:
    Vector2D velocity;
    float maxSpeed;

    VelocityComponent();
    VelocityComponent(float x, float y);
    VelocityComponent(float x, float y, float max);

    void update(float deltaTime) override;
    std::vector<uint8_t> serialize() const override;
    static VelocityComponent deserialize(const uint8_t *data);
};
