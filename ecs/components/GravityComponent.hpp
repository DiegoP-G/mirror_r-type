#pragma once
#include "../components.hpp"

class GravityComponent : public Component
{
  public:
    float gravity;
    float terminalVelocity;

    GravityComponent(float g = 800.0f, float tv = 600.0f);

    void update(float deltaTime) override;
    std::vector<uint8_t> serialize() const override;
    static GravityComponent deserialize(const uint8_t *data);
};
