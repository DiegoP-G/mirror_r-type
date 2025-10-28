#include "HealthBarComponent.hpp"
#include <cstring>
#include <stdexcept>

HealthBarComponent::HealthBarComponent(float w, float h, float oY, float oX)
    : width(w), height(h), offsetY(oY), offsetX(oX)
{
}

void HealthBarComponent::update(float deltaTime)
{
}

std::vector<uint8_t> HealthBarComponent::serialize() const
{
    std::vector<uint8_t> data;
    data.resize(sizeof(width) + sizeof(height) + sizeof(offsetY) + sizeof(offsetX));
    size_t offset = 0;

    std::memcpy(data.data() + offset, &width, sizeof(width));
    offset += sizeof(width);
    std::memcpy(data.data() + offset, &height, sizeof(height));
    offset += sizeof(height);
    std::memcpy(data.data() + offset, &offsetY, sizeof(offsetY));
    offset += sizeof(offsetY);
    std::memcpy(data.data() + offset, &offsetX, sizeof(offsetX));

    return data;
}

HealthBarComponent HealthBarComponent::deserialize(const uint8_t *data, size_t size)
{
    if (size != sizeof(float) * 4)
    {
        throw std::runtime_error("Invalid data size for HealthBarComponent deserialization");
    }

    float w, h, oY, oX;
    size_t offset = 0;

    std::memcpy(&w, data + offset, sizeof(w));
    offset += sizeof(w);
    std::memcpy(&h, data + offset, sizeof(h));
    offset += sizeof(h);
    std::memcpy(&oY, data + offset, sizeof(oY));
    offset += sizeof(oY);
    std::memcpy(&oX, data + offset, sizeof(oX));

    return HealthBarComponent(w, h, oY, oX);
}