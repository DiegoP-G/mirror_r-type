#pragma once
#include "../IComponent.hpp"

class InputComponent : public IComponent
{
  public:
    bool up, down, left, right, fire, enter;

    InputComponent();

    void update(float deltaTime) override;
    std::vector<uint8_t> serialize() const override;
    static InputComponent deserialize(const uint8_t *data, size_t size);
    bool operator==(const InputComponent &other) const;
};
