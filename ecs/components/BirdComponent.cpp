#include "BirdComponent.hpp"
#include <cstring>

BirdComponent::BirdComponent(float jump) : jumpStrength(jump), isDead(false)
{
}

void BirdComponent::update(float deltaTime)
{
}

std::vector<uint8_t> BirdComponent::serialize() const
{
    std::vector<uint8_t> data(sizeof(float) + sizeof(bool));
    std::memcpy(data.data(), &jumpStrength, sizeof(float));
    std::memcpy(data.data() + sizeof(float), &isDead, sizeof(bool));
    return data;
}

BirdComponent BirdComponent::deserialize(const uint8_t *data)
{
    BirdComponent comp;
    std::memcpy(&comp.jumpStrength, data, sizeof(float));
    std::memcpy(&comp.isDead, data + sizeof(float), sizeof(bool));
    return comp;
}
