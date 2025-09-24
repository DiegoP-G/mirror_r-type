#pragma once
#include "../components.hpp"

class JumpComponent : public Component
{
  public:
    float jumpStrength;
    bool canJump;

    JumpComponent(float jump = -400.0f);

    void update(float deltaTime) override;
    std::vector<uint8_t> serialize() const override;
    static JumpComponent deserialize(const uint8_t *data);
};
