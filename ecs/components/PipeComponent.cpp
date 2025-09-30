#include "PipeComponent.hpp"
#include <cstring>

PipeComponent::PipeComponent(bool top, float gap) : isTopPipe(top), gapHeight(gap), hasScored(false)
{
}

void PipeComponent::update(float deltaTime)
{
}

std::vector<uint8_t> PipeComponent::serialize() const
{
    std::vector<uint8_t> data(sizeof(bool) * 2 + sizeof(float));
    size_t offset = 0;

    std::memcpy(data.data() + offset, &isTopPipe, sizeof(bool));
    offset += sizeof(bool);
    std::memcpy(data.data() + offset, &gapHeight, sizeof(float));
    offset += sizeof(float);
    std::memcpy(data.data() + offset, &hasScored, sizeof(bool));
    return data;
}

PipeComponent PipeComponent::deserialize(const uint8_t *data)
{
    PipeComponent comp;
    size_t offset = 0;

    std::memcpy(&comp.isTopPipe, data + offset, sizeof(bool));
    offset += sizeof(bool);
    std::memcpy(&comp.gapHeight, data + offset, sizeof(float));
    offset += sizeof(float);
    std::memcpy(&comp.hasScored, data + offset, sizeof(bool));
    return comp;
}
