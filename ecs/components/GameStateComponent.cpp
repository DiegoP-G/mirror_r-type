#include "GameStateComponent.hpp"
#include <cstring>

GameStateComponent::GameStateComponent() : score(0), gameOver(false), started(false)
{
}

void GameStateComponent::update(float deltaTime)
{
}

std::vector<uint8_t> GameStateComponent::serialize() const
{
    std::vector<uint8_t> data(sizeof(int) + sizeof(bool) * 2);
    size_t offset = 0;

    std::memcpy(data.data() + offset, &score, sizeof(int));
    offset += sizeof(int);
    std::memcpy(data.data() + offset, &gameOver, sizeof(bool));
    offset += sizeof(bool);
    std::memcpy(data.data() + offset, &started, sizeof(bool));
    return data;
}

GameStateComponent GameStateComponent::deserialize(const uint8_t *data, size_t size)
{
    size_t expectedSize = sizeof(int) + 2 * sizeof(bool); // score + gameOver + started
    if (size < expectedSize)
    {
        throw "GameStateComponent::deserialize - données trop petites";
    }

    GameStateComponent comp;
    size_t offset = 0;

    std::memcpy(&comp.score, data + offset, sizeof(int));
    offset += sizeof(int);

    std::memcpy(&comp.gameOver, data + offset, sizeof(bool));
    offset += sizeof(bool);

    std::memcpy(&comp.started, data + offset, sizeof(bool));

    return comp;
}
