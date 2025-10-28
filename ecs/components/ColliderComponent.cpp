#include "ColliderComponent.hpp"
#include <cstring>

ColliderComponent::ColliderComponent() : isTrigger(false), isActive(true)
{
}

ColliderComponent::ColliderComponent(float w, float h) : hitbox(0, 0, w, h), isTrigger(false), isActive(true)
{
}

ColliderComponent::ColliderComponent(float w, float h, bool isAct)
    : hitbox(0, 0, w, h), isTrigger(false), isActive(isAct)
{
}

void ColliderComponent::update(float deltaTime)
{
}

std::vector<uint8_t> ColliderComponent::serialize() const
{
    std::vector<uint8_t> data;
    auto rectData = hitbox.serialize();
    data.insert(data.end(), rectData.begin(), rectData.end());
    data.push_back(static_cast<uint8_t>(isActive));
    data.push_back(static_cast<uint8_t>(isTrigger));
    return data;
}

ColliderComponent ColliderComponent::deserialize(const uint8_t *data, size_t size)
{
    size_t expectedSize = sizeof(Rect) + 2 * sizeof(bool); // hitbox + isActive + isTrigger
    if (size < expectedSize)
    {
        throw "ColliderComponent::deserialize - données trop petites";
    }

    ColliderComponent comp;
    size_t offset = 0;

    comp.hitbox = Rect::deserialize(data, sizeof(Rect));
    offset += sizeof(Rect);

    comp.isActive = static_cast<bool>(data[offset++]);
    comp.isTrigger = static_cast<bool>(data[offset]);

    return comp;
}