#pragma once
#include "../IComponent.hpp"

class PlayerComponent : public IComponent
{
  public:
    int score;
    int lives;
    float attackCooldown;
    float currentCooldown;
    int playerID;
    bool isLocal;
    bool isReady;
    bool bonusPicked = false;
    float bonusFiremode = 0;

    PlayerComponent(int playerID = 0, bool isLocal = true);

    PlayerComponent(int playerID = 0, bool isLocal = true, float shootCooldown = 0.0f);

    void update(float deltaTime) override;
    std::vector<uint8_t> serialize() const override;
    static PlayerComponent deserialize(const uint8_t *data, size_t size);
};
