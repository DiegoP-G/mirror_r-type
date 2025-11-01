#pragma once
#include "../IComponent.hpp"
#include "TransformComponent.hpp"
#include "VelocityComponent.hpp"
#include <math.h>
#define M_PI 3.14159265358979323846

class CircularMotionComponent : public IComponent
{
  public:
    float radius;
    float currentAngle;
    float angularSpeed;

    CircularMotionComponent(float radius, float initialAngle, float angularSpeed)
        : radius(radius), currentAngle(initialAngle), angularSpeed(angularSpeed * (M_PI / 180.0f))
    {
    }

    void update(float deltaTime, TransformComponent &transform, VelocityComponent &velocity);

    std::vector<uint8_t> serialize() const;
    static CircularMotionComponent deserialize(const uint8_t *data, size_t size);
};