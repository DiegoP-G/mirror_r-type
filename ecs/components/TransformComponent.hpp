#pragma once
#include "../components.hpp"

#include "Vector2D.hpp"

class TransformComponent : public Component
{
  public:
    Vector2D position;
    Vector2D scale;
    float rotation;

    TransformComponent();
    TransformComponent(float x, float y);
    TransformComponent(float x, float y, float sx, float sy, float r);

    void update(float deltaTime) override;
    std::vector<uint8_t> serialize() const override;
    static TransformComponent deserialize(const uint8_t *data);
};
