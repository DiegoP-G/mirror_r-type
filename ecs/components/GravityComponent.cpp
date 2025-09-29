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

GravityComponent GravityComponent::deserialize(const uint8_t *data)
{
    GravityComponent comp;
    std::memcpy(&comp.gravity, data, sizeof(float));
    std::memcpy(&comp.terminalVelocity, data + sizeof(float), sizeof(float));
    return comp;
}
