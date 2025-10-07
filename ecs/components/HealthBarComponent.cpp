#include "HealthBarComponent.hpp"
#include <cstring>

HealthBarComponent::HealthBarComponent(float w, float h, float oY) : width(w), height(h), offsetY(oY) {}

void HealthBarComponent::update(float deltaTime) {}

std::vector<uint8_t> HealthBarComponent::serialize() const
{
    std::vector<uint8_t> data;
    data.resize(sizeof(width) + sizeof(height) + sizeof(offsetY));
    size_t offset = 0;

    std::memcpy(data.data() + offset, &width, sizeof(width));
    offset += sizeof(width);
    std::memcpy(data.data() + offset, &height, sizeof(height));
    offset += sizeof(height);
    std::memcpy(data.data() + offset, &offsetY, sizeof(offsetY));

    return data;
}

HealthBarComponent HealthBarComponent::deserialize(const uint8_t *data, size_t size)
{
    if (size != sizeof(float) * 3)
    {
        throw "Invalid data size for HealthBarComponent deserialization";
    }

    float w, h, oY;
    size_t offset = 0;

    std::memcpy(&w, data + offset, sizeof(w));
    offset += sizeof(w);
    std::memcpy(&h, data + offset, sizeof(h));
    offset += sizeof(h);
    std::memcpy(&oY, data + offset, sizeof(oY));

    return HealthBarComponent(w, h, oY);
}