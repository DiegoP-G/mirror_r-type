#pragma once
#include "../components.hpp"
#include <vector>

#include <cstdint>

class Vector2D
{
  public:
    float x, y;

    Vector2D();
    Vector2D(float x, float y);

    Vector2D &operator+=(const Vector2D &v);
    Vector2D operator*(float scalar) const;

    std::vector<uint8_t> serialize() const;
    static Vector2D deserialize(const uint8_t *data);
};
