#include "JumpComponent.hpp"
#include <cstring>

JumpComponent::JumpComponent(float jump) : jumpStrength(jump), canJump(true)
{
}

void JumpComponent::update(float deltaTime)
{
}

std::vector<uint8_t> JumpComponent::serialize() const
{
    std::vector<uint8_t> data(sizeof(float) + sizeof(bool));
    std::memcpy(data.data(), &jumpStrength, sizeof(float));
    std::memcpy(data.data() + sizeof(float), &canJump, sizeof(bool));
    return data;
}

JumpComponent JumpComponent::deserialize(const uint8_t *data)
{
    JumpComponent comp;
    std::memcpy(&comp.jumpStrength, data, sizeof(float));
    std::memcpy(&comp.canJump, data + sizeof(float), sizeof(bool));
    return comp;
}
