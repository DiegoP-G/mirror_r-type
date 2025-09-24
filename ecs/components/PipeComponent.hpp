#pragma once
#include "../components.hpp"

class PipeComponent : public Component
{
  public:
    bool isTopPipe;
    float gapHeight;
    bool hasScored;

    PipeComponent(bool top = false, float gap = 150.0f);

    void update(float deltaTime) override;
    std::vector<uint8_t> serialize() const override;
    static PipeComponent deserialize(const uint8_t *data);
};
