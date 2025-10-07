#include "PlayerComponent.hpp"
#include <cstring>

PlayerComponent::PlayerComponent(int playerID, bool isLocal) 
    : score(0), lives(3), shootCooldown(0.0f), playerID(playerID), isLocal(isLocal), isReady(false)
{
}

void PlayerComponent::update(float deltaTime)
{
}

std::vector<uint8_t> PlayerComponent::serialize() const
{
    std::vector<uint8_t> data(sizeof(int) * 3 + sizeof(float) + sizeof(bool));
    size_t offset = 0;

    std::memcpy(data.data() + offset, &score, sizeof(int));
    offset += sizeof(int);
    std::memcpy(data.data() + offset, &lives, sizeof(int));
    offset += sizeof(int);
    std::memcpy(data.data() + offset, &shootCooldown, sizeof(float));
    offset += sizeof(float);
    std::memcpy(data.data() + offset, &playerID, sizeof(int));
    offset += sizeof(int);
    std::memcpy(data.data() + offset, &isLocal, sizeof(bool)); // <-- AJOUTER
    
    return data;
}

PlayerComponent PlayerComponent::deserialize(const uint8_t *data, size_t size)
{
    size_t expectedSize = 3 * sizeof(int) + sizeof(float) + sizeof(bool);
    if (size < expectedSize)
    {
        throw "PlayerComponent::deserialize - données trop petites";
    }

    PlayerComponent comp;
    size_t offset = 0;

    std::memcpy(&comp.score, data + offset, sizeof(int));
    offset += sizeof(int);

    std::memcpy(&comp.lives, data + offset, sizeof(int));
    offset += sizeof(int);

    std::memcpy(&comp.shootCooldown, data + offset, sizeof(float));
    offset += sizeof(float);

    std::memcpy(&comp.playerID, data + offset, sizeof(int));
    offset += sizeof(int);
    
    std::memcpy(&comp.isLocal, data + offset, sizeof(bool)); // <-- AJOUTER

    return comp;
}
