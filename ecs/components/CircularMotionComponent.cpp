#include "CircularMotionComponent.hpp"
#include <cmath>
#include <cstring>

void CircularMotionComponent::update(float deltaTime, TransformComponent &transform, VelocityComponent &velocity)
{
    // Update the angle based on angular speed
    currentAngle += angularSpeed * deltaTime;

    // Calculate the new position based on the circular motion
    transform.position.x += radius * cos(currentAngle) * deltaTime;
    transform.position.y += radius * sin(currentAngle) * deltaTime;

    // Update the velocity to reflect the circular motion
    velocity.velocity.x = -radius * angularSpeed * sin(currentAngle);
    velocity.velocity.y = radius * angularSpeed * cos(currentAngle);
}

std::vector<uint8_t> CircularMotionComponent::serialize() const
{
    std::vector<uint8_t> data(sizeof(float) * 3);
    size_t offset = 0;

    std::memcpy(data.data() + offset, &radius, sizeof(float));
    offset += sizeof(float);

    std::memcpy(data.data() + offset, &currentAngle, sizeof(float));
    offset += sizeof(float);

    std::memcpy(data.data() + offset, &angularSpeed, sizeof(float));

    return data;
}

CircularMotionComponent CircularMotionComponent::deserialize(const uint8_t *data, size_t size)
{
    size_t expectedSize = 3 * sizeof(float);
    CircularMotionComponent comp(0.0f, 0.0f, 0.0f);
    if (size < expectedSize)
    {
        return comp;
    }

    size_t offset = 0;

    std::memcpy(&comp.radius, data + offset, sizeof(float));
    offset += sizeof(float);

    std::memcpy(&comp.currentAngle, data + offset, sizeof(float));
    offset += sizeof(float);

    std::memcpy(&comp.angularSpeed, data + offset, sizeof(float));

    return comp;
}