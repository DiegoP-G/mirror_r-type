#pragma once
#include "../components.hpp"

class InputComponent : public Component
{
  public:
    bool up, down, left, right, fire;

    InputComponent();

    void update(float deltaTime) override;
    std::vector<uint8_t> serialize() const override;
    static InputComponent deserialize(const uint8_t *data);
};
