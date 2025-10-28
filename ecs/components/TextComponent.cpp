#include <cstring>
#include "TextComponent.hpp"

TextComponent::TextComponent(std::string t) : text(t)
{
}

std::vector<uint8_t> TextComponent::serialize() const
{
    std::vector<uint8_t> data;

    // Serialize the length of the string first
    size_t textLength = text.size();
    data.insert(data.end(), reinterpret_cast<const uint8_t *>(&textLength),
                reinterpret_cast<const uint8_t *>(&textLength) + sizeof(size_t));

    // Serialize the string content
    data.insert(data.end(), text.begin(), text.end());

    return data;
}

TextComponent TextComponent::deserialize(const uint8_t *data, size_t size)
{
    size_t offset = 0;

    // Deserialize the length of the string
    size_t textLength;
    std::memcpy(&textLength, data + offset, sizeof(size_t));
    offset += sizeof(size_t);

    // Deserialize the string content
    std::string text(reinterpret_cast<const char *>(data + offset), textLength);

    return TextComponent(text);
}