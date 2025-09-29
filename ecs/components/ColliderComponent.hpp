#pragma once
#include "../components.hpp"
#include "Rectangle.hpp"

class ColliderComponent : public Component
{
  public:
    Rectangle hitbox;
    bool isActive;
    bool isTrigger;

    ColliderComponent();
    ColliderComponent(float w, float h);
    ColliderComponent(float w, float h, bool isAct);

    void update(float deltaTime) override;
    std::vector<uint8_t> serialize() const override;
    static ColliderComponent deserialize(const uint8_t *data);
};
