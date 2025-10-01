#include "Vector2D.hpp"
#include <cstring>

Vector2D::Vector2D() : x(0.0f), y(0.0f)
{
}

Vector2D::Vector2D(float x, float y) : x(x), y(y)
{
}

Vector2D &Vector2D::operator+=(const Vector2D &v)
{
    x += v.x;
    y += v.y;
    return *this;
}

Vector2D Vector2D::operator*(float scalar) const
{
    return Vector2D(x * scalar, y * scalar);
}

std::vector<uint8_t> Vector2D::serialize() const
{
    std::vector<uint8_t> data(sizeof(Vector2D));
    std::memcpy(data.data(), this, sizeof(Vector2D));
    return data;
}

static Vector2D deserialize(const uint8_t *data, size_t size)
{
    if (size < sizeof(Vector2D))
        throw "Vector2D::deserialize - données trop petites";

    Vector2D vec;
    std::memcpy(&vec, data, sizeof(Vector2D));
    return vec;
}
