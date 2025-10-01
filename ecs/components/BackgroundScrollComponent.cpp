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

BackgroundScrollComponent BackgroundScrollComponent::deserialize(const uint8_t *data)
{
    BackgroundScrollComponent component;
    std::memcpy(&component.scrollSpeed, data, sizeof(float));
    std::memcpy(&component.active, data + sizeof(float), sizeof(bool));
    component.texture = reinterpret_cast<const char*>(data + sizeof(float) + sizeof(bool));
    return component;
}