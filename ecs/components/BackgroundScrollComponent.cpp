#include "BackgroundScrollComponent.hpp"
#include <cstring>

BackgroundScrollComponent::BackgroundScrollComponent(float scrollSpeed, bool active)
    : scrollSpeed(scrollSpeed), active(active)
{
}

std::vector<uint8_t> BackgroundScrollComponent::serialize() const
{
    std::vector<uint8_t> data;
    data.resize(sizeof(float) + sizeof(bool) + texture.size() + 1);
    std::memcpy(data.data(), &scrollSpeed, sizeof(float));
    std::memcpy(data.data() + sizeof(float), &active, sizeof(bool));
    std::memcpy(data.data() + sizeof(float) + sizeof(bool), texture.c_str(), texture.size() + 1);
    return data;
}

bool BackgroundScrollComponent::operator==(const BackgroundScrollComponent &other) const
{
    return scrollSpeed == other.scrollSpeed && active == other.active && texture == other.texture;
}

BackgroundScrollComponent BackgroundScrollComponent::deserialize(const uint8_t *data, size_t size)
{
    // scrollSpeed + active minimum
    size_t expectedMinSize = sizeof(float) + sizeof(bool);
    if (size < expectedMinSize)
    {
        throw "BackgroundScrollComponent::deserialize - données trop petites";
    }

    BackgroundScrollComponent component;
    size_t offset = 0;

    std::memcpy(&component.scrollSpeed, data + offset, sizeof(float));
    offset += sizeof(float);

    component.active = static_cast<bool>(data[offset++]);

    // Le reste est le nom de la texture en C-string
    if (offset < size)
    {
        component.texture = reinterpret_cast<const char *>(data + offset);
    }
    else
    {
        component.texture = nullptr;
    }

    return component;
}
