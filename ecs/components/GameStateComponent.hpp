#pragma once
#include "../components.hpp"

class GameStateComponent : public Component
{
  public:
    int score;
    bool gameOver;
    bool started;

    GameStateComponent();

    void update(float deltaTime) override;
    std::vector<uint8_t> serialize() const override;
    static GameStateComponent deserialize(const uint8_t *data);
};
