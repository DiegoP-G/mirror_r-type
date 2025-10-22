#include "ProjectileComponent.hpp"
#include <cstring>

ProjectileComponent::ProjectileComponent(float d, float lt, EntityID o, ENTITY_TYPE t)
    : damage(d), lifeTime(lt), remainingLife(lt), owner_id(o), owner_type(t)
{
}

void ProjectileComponent::update(float deltaTime)
{
    remainingLife -= deltaTime;
}

std::vector<uint8_t> ProjectileComponent::serialize() const
{
    std::vector<uint8_t> data(sizeof(float) * 3 + sizeof(EntityID) + sizeof(ENTITY_TYPE));

    size_t offset = 0;

    std::memcpy(data.data() + offset, &damage, sizeof(float));
    offset += sizeof(float);
    std::memcpy(data.data() + offset, &lifeTime, sizeof(float));
    offset += sizeof(float);
    std::memcpy(data.data() + offset, &remainingLife, sizeof(float));
    offset += sizeof(float);
    std::memcpy(data.data() + offset, &owner_id, sizeof(EntityID));
    offset += sizeof(EntityID);
    std::memcpy(data.data() + offset, &owner_type, sizeof(ENTITY_TYPE));
    return data;
}

ProjectileComponent ProjectileComponent::deserialize(const uint8_t *data, size_t size)
{
    size_t expectedSize = 3 * sizeof(float) + sizeof(EntityID);
    if (size < expectedSize)
    {
        throw "ProjectileComponent::deserialize - données trop petites";
    }

    ProjectileComponent comp(0, 0, 0, PLAYER);
    size_t offset = 0;

    std::memcpy(&comp.damage, data + offset, sizeof(float));
    offset += sizeof(float);

    std::memcpy(&comp.lifeTime, data + offset, sizeof(float));
    offset += sizeof(float);

    std::memcpy(&comp.remainingLife, data + offset, sizeof(float));
    offset += sizeof(float);

    std::memcpy(&comp.owner_id, data + offset, sizeof(EntityID));
    offset += sizeof(EntityID);

    std::memcpy(&comp.owner_type, data + offset, sizeof(ENTITY_TYPE));

    return comp;
}
