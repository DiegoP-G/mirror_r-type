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

bool CenteredComponent::operator==(const CenteredComponent &other) const
{
    return offsetX == other.offsetX && offsetY == other.offsetY;
}

CenteredComponent CenteredComponent::deserialize(const uint8_t *data, size_t size)
{
    size_t expectedSize = 2 * sizeof(float); // offsetX + offsetY
    if (size < expectedSize)
    {
        throw "CenteredComponent::deserialize - données trop petites";
    }

    CenteredComponent comp;
    size_t offset = 0;

    std::memcpy(&comp.offsetX, data + offset, sizeof(float));
    offset += sizeof(float);

    std::memcpy(&comp.offsetY, data + offset, sizeof(float));

    return comp;
}
