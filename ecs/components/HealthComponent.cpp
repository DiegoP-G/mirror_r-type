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

HealthComponent HealthComponent::deserialize(const uint8_t *data, size_t size)
{
    size_t expectedSize = 2 * sizeof(float); // health + maxHealth
    if (size < expectedSize)
    {
        throw "HealthComponent::deserialize - données trop petites";
    }

    HealthComponent comp(0);
    size_t offset = 0;

    std::memcpy(&comp.health, data + offset, sizeof(float));
    offset += sizeof(float);

    std::memcpy(&comp.maxHealth, data + offset, sizeof(float));

    return comp;
}
