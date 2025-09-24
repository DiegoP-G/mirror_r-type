#include "SpriteComponent.hpp"
#include <cstring>

SpriteComponent::SpriteComponent() : texture(nullptr), isVisible(true), width(32), height(32), r(255), g(255), b(255), a(255)
{
}

SpriteComponent::SpriteComponent(int w, int h, uint8_t red, uint8_t green, uint8_t blue)
    : texture(nullptr), isVisible(true), width(w), height(h), r(red), g(green), b(blue), a(255)
{
}

void SpriteComponent::update(float deltaTime)
{
}

void SpriteComponent::render()
{
}

std::vector<uint8_t> SpriteComponent::serialize() const
{
    std::vector<uint8_t> data;
    auto rectData = srcRect.serialize();
    data.insert(data.end(), rectData.begin(), rectData.end());

    data.insert(data.end(), reinterpret_cast<const uint8_t *>(&width),
                reinterpret_cast<const uint8_t *>(&width) + sizeof(int));
    data.insert(data.end(), reinterpret_cast<const uint8_t *>(&height),
                reinterpret_cast<const uint8_t *>(&height) + sizeof(int));
    data.push_back(static_cast<uint8_t>(isVisible));
    data.push_back(r);
    data.push_back(g);
    data.push_back(b);
    data.push_back(a);
    return data;
}

SpriteComponent SpriteComponent::deserialize(const uint8_t *data)
{
    SpriteComponent comp;
    comp.srcRect = Rectangle::deserialize(data);
    size_t offset = sizeof(Rectangle);

    std::memcpy(&comp.width, data + offset, sizeof(int));
    offset += sizeof(int);
    std::memcpy(&comp.height, data + offset, sizeof(int));
    offset += sizeof(int);
    comp.isVisible = static_cast<bool>(data[offset++]);
    comp.r = data[offset++];
    comp.g = data[offset++];
    comp.b = data[offset++];
    comp.a = data[offset];
    comp.texture = nullptr;
    return comp;
}
