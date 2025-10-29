// ! PlayerComponent.hpp addition
#pragma once
#include "../IComponent.hpp"
#include <iostream>

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
    std::string name;
    bool bonusPicked = false;
    float bonusFiremode = 0;

    float stamina;
    float maxStamina;
    float staminaRegenRate;
    float moveSpeed;

    PlayerComponent(int playerID = 0, bool isLocal = true, std::string username = "");
    PlayerComponent(int playerID = 0, bool isLocal = true, float shootCooldown = 0.0f, std::string username = "");

    void update(float deltaTime) override;
    std::vector<uint8_t> serialize() const override;
    static PlayerComponent deserialize(const uint8_t *data, size_t size);
};
