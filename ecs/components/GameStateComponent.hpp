#pragma once
#include "../IComponent.hpp"

class GameStateComponent : public IComponent
{
  public:
    int score;
    bool gameOver;
    bool started;

    GameStateComponent();

    void update(float deltaTime) override;
    std::vector<uint8_t> serialize() const override;
    static GameStateComponent deserialize(const uint8_t *data, size_t size);
};
