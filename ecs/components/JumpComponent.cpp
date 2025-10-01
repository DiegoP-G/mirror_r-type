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

JumpComponent JumpComponent::deserialize(const uint8_t *data, size_t size)
{
    size_t expectedSize = sizeof(float) + sizeof(bool); // jumpStrength + canJump
    if (size < expectedSize)
    {
        throw "JumpComponent::deserialize - données trop petites";
    }

    JumpComponent comp;
    size_t offset = 0;

    std::memcpy(&comp.jumpStrength, data + offset, sizeof(float));
    offset += sizeof(float);

    std::memcpy(&comp.canJump, data + offset, sizeof(bool));

    return comp;
}
