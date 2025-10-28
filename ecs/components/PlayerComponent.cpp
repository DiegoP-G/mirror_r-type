#include "PlayerComponent.hpp"
#include <cstring>

PlayerComponent::PlayerComponent(int playerID, bool isLocal, std::string username)
    : score(0), lives(3), attackCooldown(0.0f), currentCooldown(0.0f), playerID(playerID), isLocal(isLocal),
      isReady(false), name(username), stamina(100.0f), // ! added: initial stamina
      maxStamina(100.0f),                              // ! added: max stamina
      staminaRegenRate(15.0f),                         // ! added: regen rate
      warpCooldown(0.0f)                               // ! added: warp cooldown
{
}

PlayerComponent::PlayerComponent(int playerID, bool isLocal, float attackCooldown, std::string username)
    : score(0), lives(3), attackCooldown(attackCooldown), currentCooldown(0.0f), playerID(playerID), isLocal(isLocal),
      isReady(false), name(username), stamina(100.0f), // ! added
      maxStamina(100.0f),                              // ! added
      staminaRegenRate(15.0f),                         // ! added
      warpCooldown(0.0f)                               // ! added
{
}

void PlayerComponent::update(float deltaTime)
{
}

std::vector<uint8_t> PlayerComponent::serialize() const
{
    size_t nameLength = name.size();

    // ! updated size to include stamina data and warpCooldown
    std::vector<uint8_t> data(sizeof(int) * 3 + sizeof(float) * 5 + sizeof(bool) + sizeof(size_t) + nameLength);
    size_t offset = 0;

    std::memcpy(data.data() + offset, &score, sizeof(int));
    offset += sizeof(int);
    std::memcpy(data.data() + offset, &lives, sizeof(int));
    offset += sizeof(int);
    std::memcpy(data.data() + offset, &attackCooldown, sizeof(float));
    offset += sizeof(float);
    std::memcpy(data.data() + offset, &playerID, sizeof(int));
    offset += sizeof(int);
    std::memcpy(data.data() + offset, &isLocal, sizeof(bool));
    offset += sizeof(bool);

    // ! added serialization for stamina values
    std::memcpy(data.data() + offset, &stamina, sizeof(float));
    offset += sizeof(float);
    std::memcpy(data.data() + offset, &maxStamina, sizeof(float));
    offset += sizeof(float);
    std::memcpy(data.data() + offset, &staminaRegenRate, sizeof(float));
    offset += sizeof(float);
    std::memcpy(data.data() + offset, &warpCooldown, sizeof(float));
    offset += sizeof(float);

    std::memcpy(data.data() + offset, &nameLength, sizeof(size_t));
    offset += sizeof(size_t);
    std::memcpy(data.data() + offset, name.data(), nameLength);

    return data;
}

PlayerComponent PlayerComponent::deserialize(const uint8_t *data, size_t size)
{
    size_t offset = 0;

    // ! updated expected size for new stamina fields
    size_t expectedFixedSize = 3 * sizeof(int) + sizeof(float) * 5 + sizeof(bool) + sizeof(size_t);
    if (size < expectedFixedSize)
        throw "PlayerComponent::deserialize - data too small";

    PlayerComponent comp(0, true, 0.0f);

    std::memcpy(&comp.score, data + offset, sizeof(int));
    offset += sizeof(int);
    std::memcpy(&comp.lives, data + offset, sizeof(int));
    offset += sizeof(int);
    std::memcpy(&comp.attackCooldown, data + offset, sizeof(float));
    offset += sizeof(float);
    std::memcpy(&comp.playerID, data + offset, sizeof(int));
    offset += sizeof(int);
    std::memcpy(&comp.isLocal, data + offset, sizeof(bool));
    offset += sizeof(bool);

    // ! added deserialization for stamina fields
    std::memcpy(&comp.stamina, data + offset, sizeof(float));
    offset += sizeof(float);
    std::memcpy(&comp.maxStamina, data + offset, sizeof(float));
    offset += sizeof(float);
    std::memcpy(&comp.staminaRegenRate, data + offset, sizeof(float));
    offset += sizeof(float);
    std::memcpy(&comp.warpCooldown, data + offset, sizeof(float));
    offset += sizeof(float);

    size_t nameLength;
    std::memcpy(&nameLength, data + offset, sizeof(size_t));
    offset += sizeof(size_t);

    if (size < offset + nameLength)
        throw "PlayerComponent::deserialize - insufficient data for name";

    comp.name = std::string(reinterpret_cast<const char *>(data + offset), nameLength);

    return comp;
}
