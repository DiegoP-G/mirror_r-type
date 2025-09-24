#pragma once
#include "../components.hpp"

class HealthComponent : public Component
{
  public:
    float health;
    float maxHealth;

    HealthComponent(float h);

    void update(float deltaTime) override;
    std::vector<uint8_t> serialize() const override;
    static HealthComponent deserialize(const uint8_t *data);
};
