#pragma once
#include "../components.hpp"

class PlayerComponent : public Component
{
  public:
    int score;
    int lives;
    float shootCooldown;
    int playerID;

    PlayerComponent(int playerID = 0);

    void update(float deltaTime) override;
    std::vector<uint8_t> serialize() const override;
    static PlayerComponent deserialize(const uint8_t *data);
};
