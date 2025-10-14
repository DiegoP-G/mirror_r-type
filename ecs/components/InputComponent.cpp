#include "InputComponent.hpp"

InputComponent::InputComponent() : up(false), down(false), left(false), right(false), fire(false), enter(false)
{
}

void InputComponent::update(float deltaTime)
{
}

std::vector<uint8_t> InputComponent::serialize() const
{
    std::vector<uint8_t> data(6);
    data[0] = static_cast<uint8_t>(up);
    data[1] = static_cast<uint8_t>(down);
    data[2] = static_cast<uint8_t>(left);
    data[3] = static_cast<uint8_t>(right);
    data[4] = static_cast<uint8_t>(fire);
    data[5] = static_cast<uint8_t>(enter);
    return data;
}

bool InputComponent::operator==(const InputComponent &other) const
{
    return up == other.up && down == other.down && left == other.left && right == other.right && fire == other.fire &&
           enter == other.enter;
}

InputComponent InputComponent::deserialize(const uint8_t *data, size_t size)
{
    size_t expectedSize = 6; // up, down, left, right, fire, enter
    if (size < expectedSize)
    {
        throw "InputComponent::deserialize - données trop petites";
    }

    InputComponent comp;
    comp.up = static_cast<bool>(data[0]);
    comp.down = static_cast<bool>(data[1]);
    comp.left = static_cast<bool>(data[2]);
    comp.right = static_cast<bool>(data[3]);
    comp.fire = static_cast<bool>(data[4]);
    comp.enter = static_cast<bool>(data[5]);

    return comp;
}
