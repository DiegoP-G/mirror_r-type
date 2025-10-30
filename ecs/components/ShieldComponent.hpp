#pragma once
#include "../IComponent.hpp"
#include "Vector2D.hpp"

class ShieldComponent : public IComponent
{
  public:
    int ownerID;
    float shieldLeft = 0;

    ShieldComponent();
    ShieldComponent(int ownerID);

    void update(float deltaTime) override;
    std::vector<uint8_t> serialize() const override;
    static ShieldComponent deserialize(const uint8_t *data, size_t size);
};
