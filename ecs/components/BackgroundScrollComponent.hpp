#pragma once
#include "../components.hpp"
#include <string>

class BackgroundScrollComponent : public Component
{
  public:
    float scrollSpeed = 0.0f;
    bool active = false;
    std::string texture;

    BackgroundScrollComponent() = default;
    BackgroundScrollComponent(float scrollSpeed, bool active);
    std::vector<uint8_t> serialize() const override;
    static BackgroundScrollComponent deserialize(const uint8_t *data);
};
