#include "LaserWarningComponent.hpp"
#include <cstring>

LaserWarningComponent::LaserWarningComponent(float w, float h, float app, float warn, float active)
    : width(w), height(h), appearanceTime(app), warningTime(warn), activeTime(active), isActive(false),
      warningShown(false)
{
}

void LaserWarningComponent::update(float deltaTime)
{
}

std::vector<uint8_t> LaserWarningComponent::serialize() const
{
    std::vector<uint8_t> data(sizeof(float) * 5 + sizeof(bool) * 2);
    size_t offset = 0;

    std::memcpy(data.data() + offset, &appearanceTime, sizeof(float));
    offset += sizeof(float);
    std::memcpy(data.data() + offset, &warningTime, sizeof(float));
    offset += sizeof(float);
    std::memcpy(data.data() + offset, &activeTime, sizeof(float));
    offset += sizeof(float);
    std::memcpy(data.data() + offset, &isActive, sizeof(bool));
    offset += sizeof(bool);
    std::memcpy(data.data() + offset, &warningShown, sizeof(bool));
    offset += sizeof(bool);
    std::memcpy(data.data() + offset, &width, sizeof(float));
    offset += sizeof(float);
    std::memcpy(data.data() + offset, &height, sizeof(float));
    return data;
}

LaserWarningComponent LaserWarningComponent::deserialize(const uint8_t *data)
{
    LaserWarningComponent comp;
    size_t offset = 0;

    std::memcpy(&comp.appearanceTime, data + offset, sizeof(float));
    offset += sizeof(float);
    std::memcpy(&comp.warningTime, data + offset, sizeof(float));
    offset += sizeof(float);
    std::memcpy(&comp.activeTime, data + offset, sizeof(float));
    offset += sizeof(float);
    std::memcpy(&comp.isActive, data + offset, sizeof(bool));
    offset += sizeof(bool);
    std::memcpy(&comp.warningShown, data + offset, sizeof(bool));
    offset += sizeof(bool);
    std::memcpy(&comp.width, data + offset, sizeof(float));
    offset += sizeof(float);
    std::memcpy(&comp.height, data + offset, sizeof(float));
    return comp;
}
