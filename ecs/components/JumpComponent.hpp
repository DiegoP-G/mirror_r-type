#pragma once
#include "../IComponent.hpp"

class JumpComponent : public IComponent
{
  public:
    float jumpStrength;
    bool canJump;

    JumpComponent(float jump = -400.0f);

    void update(float deltaTime) override;
    std::vector<uint8_t> serialize() const override;
    static JumpComponent deserialize(const uint8_t *data, size_t size);
};
