#include "ProjectileComponent.hpp"
#include <cstring>

ProjectileComponent::ProjectileComponent(float d, float lt, EntityID o)
    : damage(d), lifeTime(lt), remainingLife(lt), owner(o)
{
}

void ProjectileComponent::update(float deltaTime)
{
    remainingLife -= deltaTime;
}

std::vector<uint8_t> ProjectileComponent::serialize() const
{
    std::vector<uint8_t> data(sizeof(float) * 3 + sizeof(EntityID));
    size_t offset = 0;

    std::memcpy(data.data() + offset, &damage, sizeof(float));
    offset += sizeof(float);
    std::memcpy(data.data() + offset, &lifeTime, sizeof(float));
    offset += sizeof(float);
    std::memcpy(data.data() + offset, &remainingLife, sizeof(float));
    offset += sizeof(float);
    std::memcpy(data.data() + offset, &owner, sizeof(EntityID));
    return data;
}

ProjectileComponent ProjectileComponent::deserialize(const uint8_t *data, size_t size)
{
    size_t expectedSize = 3 * sizeof(float) + sizeof(EntityID);
    if (size < expectedSize)
    {
        throw "ProjectileComponent::deserialize - données trop petites";
    }

    ProjectileComponent comp(0, 0, 0);
    size_t offset = 0;

    std::memcpy(&comp.damage, data + offset, sizeof(float));
    offset += sizeof(float);

    std::memcpy(&comp.lifeTime, data + offset, sizeof(float));
    offset += sizeof(float);

    std::memcpy(&comp.remainingLife, data + offset, sizeof(float));
    offset += sizeof(float);

    std::memcpy(&comp.owner, data + offset, sizeof(EntityID));

    return comp;
}
