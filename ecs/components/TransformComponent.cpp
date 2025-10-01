#include "TransformComponent.hpp"
#include <cstring>

TransformComponent::TransformComponent() : position(0, 0), scale(1, 1), rotation(0)
{
}

TransformComponent::TransformComponent(float x, float y) : position(x, y), scale(1, 1), rotation(0)
{
}

TransformComponent::TransformComponent(float x, float y, float sx, float sy, float r)
    : position(x, y), scale(sx, sy), rotation(r)
{
}

void TransformComponent::update(float deltaTime)
{
}

std::vector<uint8_t> TransformComponent::serialize() const
{
    std::vector<uint8_t> data;
    auto posData = position.serialize();
    auto scaleData = scale.serialize();

    data.insert(data.end(), posData.begin(), posData.end());
    data.insert(data.end(), scaleData.begin(), scaleData.end());
    data.insert(data.end(), reinterpret_cast<const uint8_t *>(&rotation),
                reinterpret_cast<const uint8_t *>(&rotation) + sizeof(float));
    return data;
}

TransformComponent TransformComponent::deserialize(const uint8_t *data, size_t size)
{
    size_t expectedSize = 2 * sizeof(Vector2D) + sizeof(float);
    if (size < expectedSize)
    {
        throw "TransformComponent::deserialize - données trop petites";
    }

    TransformComponent comp;
    comp.position = Vector2D::deserialize(data, sizeof(Vector2D));
    comp.scale = Vector2D::deserialize(data + sizeof(Vector2D), sizeof(Vector2D));
    std::memcpy(&comp.rotation, data + 2 * sizeof(Vector2D), sizeof(float));
    return comp;
}
