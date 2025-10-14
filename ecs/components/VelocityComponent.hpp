#pragma once
#include "../IComponent.hpp"
#include "Vector2D.hpp"

class VelocityComponent : public IComponent
{
  public:
    Vector2D velocity;
    float maxSpeed;
    bool sineMovement;
    bool reverseSineMovement;
    float time;

    VelocityComponent();
    VelocityComponent(float x, float y, bool sine = false, bool reverseSine = false);
    VelocityComponent(float x, float y, float max, bool sine = false, bool reverseSine = false);

    void update(float deltaTime) override;
    std::vector<uint8_t> serialize() const override;
    static VelocityComponent deserialize(const uint8_t *data, size_t size);

    bool operator==(const VelocityComponent &other) const;
};
