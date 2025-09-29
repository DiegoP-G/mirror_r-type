#include "Rectangle.hpp"
#include <cstring>

Rectangle::Rectangle() : x(0), y(0), w(0), h(0)
{
}

Rectangle::Rectangle(float x, float y, float w, float h) : x(x), y(y), w(w), h(h)
{
}

bool Rectangle::intersects(const Rectangle &other) const
{
    return !(x + w < other.x || other.x + other.w < x || y + h < other.y || other.y + other.h < y);
}

std::vector<uint8_t> Rectangle::serialize() const
{
    std::vector<uint8_t> data(sizeof(Rectangle));
    std::memcpy(data.data(), this, sizeof(Rectangle));
    return data;
}

Rectangle Rectangle::deserialize(const uint8_t *data)
{
    Rectangle rect;
    std::memcpy(&rect, data, sizeof(Rectangle));
    return rect;
}
