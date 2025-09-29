#pragma once
#include "../components.hpp"

#include <cstdint>
#include <vector>

class Rectangle
{
  public:
    float x, y, w, h;

    Rectangle();
    Rectangle(float x, float y, float w, float h);

    bool intersects(const Rectangle &other) const;
    std::vector<uint8_t> serialize() const;
    static Rectangle deserialize(const uint8_t *data);
};
