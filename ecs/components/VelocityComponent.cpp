#include "VelocityComponent.hpp"
#include <cstring>

VelocityComponent::VelocityComponent() : velocity(0, 0), maxSpeed(5.0f)
{
}

VelocityComponent::VelocityComponent(float x, float y) : velocity(x, y), maxSpeed(5.0f)
{
}

VelocityComponent::VelocityComponent(float x, float y, float max) : velocity(x, y), maxSpeed(max)
{
}

void VelocityComponent::update(float deltaTime)
{
}

std::vector<uint8_t> VelocityComponent::serialize() const
{
    std::vector<uint8_t> data;
    auto velData = velocity.serialize();
    data.insert(data.end(), velData.begin(), velData.end());
    data.insert(data.end(), reinterpret_cast<const uint8_t *>(&maxSpeed),
                reinterpret_cast<const uint8_t *>(&maxSpeed) + sizeof(float));
    return data;
}

VelocityComponent VelocityComponent::deserialize(const uint8_t *data, size_t size)
{
    if (size < sizeof(Vector2D) + sizeof(float))
    {
        throw "VelocityComponent::deserialize - données trop petites";
    }

    VelocityComponent comp;
    comp.velocity = Vector2D::deserialize(data, sizeof(Vector2D));
    std::memcpy(&comp.maxSpeed, data + sizeof(Vector2D), sizeof(float));
    return comp;
}
