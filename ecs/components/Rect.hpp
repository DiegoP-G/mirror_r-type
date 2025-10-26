#pragma once

#include <cstdint>
#include <vector>

class Rect
{
  public:
    float x, y, w, h;

    Rect();
    Rect(float x, float y, float w, float h);

    bool intersects(const Rect &other) const;
    std::vector<uint8_t> serialize() const;
    static Rect deserialize(const uint8_t *data, size_t size);
};
