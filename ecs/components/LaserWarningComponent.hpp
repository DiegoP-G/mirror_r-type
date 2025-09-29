#pragma once
#include "../components.hpp"

class LaserWarningComponent : public Component
{
  public:
    float appearanceTime;
    float warningTime;
    float activeTime;
    bool isActive;
    bool warningShown;
    float width;
    float height;

    LaserWarningComponent(float w = 10.0f, float h = 300.0f, float app = 1.0f, float warn = 1.0f, float active = 1.0f);

    void update(float deltaTime) override;
    std::vector<uint8_t> serialize() const override;
    static LaserWarningComponent deserialize(const uint8_t *data);
};
