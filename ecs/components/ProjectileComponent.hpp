#pragma once
#include "../IComponent.hpp"
#include "../entity.hpp"
#include "../using.hpp"

class ProjectileComponent : public IComponent
{
  public:
    float damage;
    float lifeTime;
    float remainingLife;
    EntityID owner_id;
    ENTITY_TYPE owner_type;

    ProjectileComponent(float d, float lt, EntityID o, ENTITY_TYPE t);

    void update(float deltaTime) override;
    std::vector<uint8_t> serialize() const override;
    static ProjectileComponent deserialize(const uint8_t *data, size_t size);

    bool operator==(const ProjectileComponent &other) const;
};
