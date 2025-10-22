#include "Rect.hpp"
#include <cstring>

Rect::Rect() : x(0), y(0), w(0), h(0)
{
}

Rect::Rect(float x, float y, float w, float h) : x(x), y(y), w(w), h(h)
{
}

bool Rect::intersects(const Rect &other) const
{
    return !(x + w < other.x || other.x + other.w < x || y + h < other.y || other.y + other.h < y);
}

std::vector<uint8_t> Rect::serialize() const
{
    std::vector<uint8_t> data(sizeof(Rect));
    std::memcpy(data.data(), this, sizeof(Rect));
    return data;
}

Rect Rect::deserialize(const uint8_t *data, size_t size)
{
    if (size < sizeof(Rect))
    {
        throw "Rectangle::deserialize - données trop petites";
    }

    Rect rect;
    std::memcpy(&rect, data, sizeof(Rect));
    return rect;
}
