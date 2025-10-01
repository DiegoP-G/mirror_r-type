#include "EnemyComponent.hpp"
#include <cstring>

EnemyComponent::EnemyComponent(int type, float attackCooldown, int shootingType)
    : type(type), shootingType(shootingType), attackCooldown(attackCooldown), currentCooldown(0)
{
}

void EnemyComponent::update(float deltaTime)
{
    if (currentCooldown > 0)
    {
        currentCooldown -= deltaTime;
    }
}

std::vector<uint8_t> EnemyComponent::serialize() const
{
    std::vector<uint8_t> data(sizeof(int) * 2 + sizeof(float) * 2);
    size_t offset = 0;

    std::memcpy(data.data() + offset, &type, sizeof(int));
    offset += sizeof(int);
    std::memcpy(data.data() + offset, &shootingType, sizeof(int));
    offset += sizeof(int);
    std::memcpy(data.data() + offset, &attackCooldown, sizeof(float));
    offset += sizeof(float);
    std::memcpy(data.data() + offset, &currentCooldown, sizeof(float));
    return data;
}

EnemyComponent EnemyComponent::deserialize(const uint8_t *data)
{
    EnemyComponent comp(0, 0, 0);
    size_t offset = 0;

    std::memcpy(&comp.type, data + offset, sizeof(int));
    offset += sizeof(int);
    std::memcpy(&comp.shootingType, data + offset, sizeof(int));
    offset += sizeof(int);
    std::memcpy(&comp.attackCooldown, data + offset, sizeof(float));
    offset += sizeof(float);
    std::memcpy(&comp.currentCooldown, data + offset, sizeof(float));
    return comp;
}
