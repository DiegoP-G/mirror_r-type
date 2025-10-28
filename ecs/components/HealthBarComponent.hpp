#pragma once
#include "../IComponent.hpp"

class HealthBarComponent : public IComponent
{
  public:
    float width;
    float height;
    float offsetY;
    float offsetX;

    HealthBarComponent(float w, float h, float oY, float oX = 0.0f);

    void update(float deltaTime) override;
    std::vector<uint8_t> serialize() const override;
    static HealthBarComponent deserialize(const uint8_t *data, size_t size);
};