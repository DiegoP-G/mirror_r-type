#pragma once
#include "../IComponent.hpp"

class PlayerComponent : public IComponent
{
  public:
    int score;
    int lives;
    float shootCooldown;
    int playerID;
    bool isLocal;
    bool isReady;

    PlayerComponent(int playerID = 0, bool isLocal = true);

    void update(float deltaTime) override;
    std::vector<uint8_t> serialize() const override;
    static PlayerComponent deserialize(const uint8_t *data, size_t size);
};
