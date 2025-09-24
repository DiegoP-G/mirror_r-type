#include "HealthComponent.hpp"
#include <cstring>

HealthComponent::HealthComponent(float h) : health(h), maxHealth(h)
{
}

void HealthComponent::update(float deltaTime)
{
}

std::vector<uint8_t> HealthComponent::serialize() const
{
    std::vector<uint8_t> data(sizeof(float) * 2);
    std::memcpy(data.data(), &health, sizeof(float));
    std::memcpy(data.data() + sizeof(float), &maxHealth, sizeof(float));
    return data;
}

HealthComponent HealthComponent::deserialize(const uint8_t *data)
{
    HealthComponent comp(0);
    std::memcpy(&comp.health, data, sizeof(float));
    std::memcpy(&comp.maxHealth, data + sizeof(float), sizeof(float));
    return comp;
}
