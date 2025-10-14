#pragma once
#include "../IComponent.hpp"

class CenteredComponent : public IComponent
{
  public:
    float offsetX;
    float offsetY;

    CenteredComponent(float x = 0, float y = 0);

    void update(float deltaTime) override;
    std::vector<uint8_t> serialize() const override;
    static CenteredComponent deserialize(const uint8_t *data, size_t size);
    bool operator==(const CenteredComponent &other) const;
};
