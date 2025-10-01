#pragma once
#include "../IComponent.hpp"
#include "../using.hpp"

class ProjectileComponent : public IComponent
{
  public:
    float damage;
    float lifeTime;
    float remainingLife;
    EntityID owner;

    ProjectileComponent(float d, float lt, EntityID o);

    void update(float deltaTime) override;
    std::vector<uint8_t> serialize() const override;
    static ProjectileComponent deserialize(const uint8_t *data, size_t size);
};
