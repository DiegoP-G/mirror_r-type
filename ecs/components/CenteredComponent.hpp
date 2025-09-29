#pragma once
#include "../components.hpp"

class CenteredComponent : public Component
{
  public:
    float offsetX;
    float offsetY;

    CenteredComponent(float x = 0, float y = 0);

    void update(float deltaTime) override;
    std::vector<uint8_t> serialize() const override;
    static CenteredComponent deserialize(const uint8_t *data);
};
