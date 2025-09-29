#include "CenteredComponent.hpp"
#include <cstring>

CenteredComponent::CenteredComponent(float x, float y) : offsetX(x), offsetY(y)
{
}

void CenteredComponent::update(float deltaTime)
{
}

std::vector<uint8_t> CenteredComponent::serialize() const
{
    std::vector<uint8_t> data(sizeof(float) * 2);
    std::memcpy(data.data(), &offsetX, sizeof(float));
    std::memcpy(data.data() + sizeof(float), &offsetY, sizeof(float));
    return data;
}

CenteredComponent CenteredComponent::deserialize(const uint8_t *data)
{
    CenteredComponent comp;
    std::memcpy(&comp.offsetX, data, sizeof(float));
    std::memcpy(&comp.offsetY, data + sizeof(float), sizeof(float));
    return comp;
}
