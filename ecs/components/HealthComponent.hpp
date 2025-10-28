#pragma once
#include "../IComponent.hpp"

class HealthComponent : public IComponent
{
  public:
    float health;
    float maxHealth;

    HealthComponent(float h, float hMax) : health(h), maxHealth(hMax) {};

    HealthComponent(float h) : health(h), maxHealth(h) {};

    void update(float deltaTime) override;
    std::vector<uint8_t> serialize() const override;
    static HealthComponent deserialize(const uint8_t *data, size_t size);
};
