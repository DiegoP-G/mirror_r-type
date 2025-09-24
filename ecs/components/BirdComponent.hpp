#pragma once
#include "../components.hpp"

class BirdComponent : public Component
{
  public:
    float jumpStrength;
    bool isDead;

    BirdComponent(float jump = -400.0f);

    void update(float deltaTime) override;
    std::vector<uint8_t> serialize() const override;
    static BirdComponent deserialize(const uint8_t *data);
};
