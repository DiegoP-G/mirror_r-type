#include "GravityComponent.hpp"
#include <cstring>

GravityComponent::GravityComponent(float g, float tv) : gravity(g), terminalVelocity(tv)
{
}

void GravityComponent::update(float deltaTime)
{
}

std::vector<uint8_t> GravityComponent::serialize() const
{
    std::vector<uint8_t> data(sizeof(float) * 2);
    std::memcpy(data.data(), &gravity, sizeof(float));
    std::memcpy(data.data() + sizeof(float), &terminalVelocity, sizeof(float));
    return data;
}

GravityComponent GravityComponent::deserialize(const uint8_t *data, size_t size)
{
    size_t expectedSize = 2 * sizeof(float); // gravity + terminalVelocity
    if (size < expectedSize)
    {
        throw "GravityComponent::deserialize - données trop petites";
    }

    GravityComponent comp;
    size_t offset = 0;

    std::memcpy(&comp.gravity, data + offset, sizeof(float));
    offset += sizeof(float);

    std::memcpy(&comp.terminalVelocity, data + offset, sizeof(float));

    return comp;
}
