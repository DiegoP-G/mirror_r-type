#pragma once
#include "../IComponent.hpp"
#include "Vector2D.hpp"
#include <iostream>

class TextComponent : public IComponent
{
  public:
    std::string text;

    TextComponent(std::string text);

    std::vector<uint8_t> serialize() const override;
    static TextComponent deserialize(const uint8_t *data, size_t size);
};