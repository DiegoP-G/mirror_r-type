#include "PlayerComponent.hpp"
#include <cstring>

PlayerComponent::PlayerComponent(int playerID, bool isLocal, std::string username)
    : score(0), lives(3), attackCooldown(0.0f), currentCooldown(0.0f), playerID(playerID), isLocal(isLocal),
      isReady(false), name(username)
{
}

PlayerComponent::PlayerComponent(int playerID, bool isLocal, float attackCooldown, std::string username)
    : score(0), lives(3), attackCooldown(attackCooldown), currentCooldown(0.0f), playerID(playerID), isLocal(isLocal),
      isReady(false), name(username)
{
}

void PlayerComponent::update(float deltaTime)
{
}

std::vector<uint8_t> PlayerComponent::serialize() const
{
    size_t nameLength = name.size();
    std::vector<uint8_t> data(sizeof(int) * 3 + sizeof(float) + sizeof(bool) + sizeof(size_t) + nameLength);
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

    std::memcpy(data.data() + offset, &nameLength, sizeof(size_t));
    offset += sizeof(size_t);
    std::memcpy(data.data() + offset, name.data(), nameLength);

    return data;
}

PlayerComponent PlayerComponent::deserialize(const uint8_t *data, size_t size)
{
    size_t offset = 0;

    size_t expectedFixedSize = 3 * sizeof(int) + sizeof(float) + sizeof(bool) + sizeof(size_t);
    if (size < expectedFixedSize)
    {
        throw "PlayerComponent::deserialize - données trop petites";
    }

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

    size_t nameLength;
    std::memcpy(&nameLength, data + offset, sizeof(size_t));
    offset += sizeof(size_t);

    if (size < offset + nameLength)
    {
        throw "PlayerComponent::deserialize - données trop petites pour le nom";
    }

    comp.name = std::string(reinterpret_cast<const char *>(data + offset), nameLength);

    return comp;
}
