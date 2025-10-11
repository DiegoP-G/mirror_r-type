#pragma once
#include "../IComponent.hpp"

class EnemyComponent : public IComponent
{
  public:
    int type;
    int shootingType;
    float attackCooldown;
    float currentCooldown;
    int scoreValue;

    EnemyComponent(int t, float ac, int st, int scoreValue);

    void update(float deltaTime) override;
    std::vector<uint8_t> serialize() const override;
    static EnemyComponent deserialize(const uint8_t *data, size_t size);
};
