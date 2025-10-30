#include "ShieldComponent.hpp"
#include <cstring>
#include <stdexcept>

ShieldComponent::ShieldComponent()
{
}

ShieldComponent::ShieldComponent(int ownerID)
    : ownerID(ownerID)
{
}

void ShieldComponent::update(float deltaTime)
{
}

std::vector<uint8_t> ShieldComponent::serialize() const
{
    std::vector<uint8_t> data(sizeof(int) + sizeof(float));

    std::memcpy(data.data(), &ownerID, sizeof(int));
    std::memcpy(data.data() + sizeof(int), &shieldLeft, sizeof(float));
    
    return data;
}

ShieldComponent ShieldComponent::deserialize(const uint8_t *data, size_t size)
{
    size_t expectedSize = sizeof(int) + sizeof(float);
    if (size < expectedSize)
    {
        throw std::runtime_error("Invalid data size for ShieldComponent deserialization");
    }

    ShieldComponent comp;
    std::memcpy(&comp.ownerID, data, sizeof(int));
    std::memcpy(&comp.shieldLeft, data + sizeof(int), sizeof(float));
    return comp;
}
