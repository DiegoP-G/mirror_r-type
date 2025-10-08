/*
** EPITECH PROJECT, 2025
** mirror_r-type
** File description:
** BonusComponent
*/

#include "BonusComponent.hpp"
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <tuple>
#include <vector>

BonusComponent::BonusComponent(std::vector<std::tuple<TypeBonus, int>> b) : bonus(std::move(b))
{
}

void BonusComponent::update(float deltyatime)
{
}

std::vector<uint8_t> BonusComponent::serialize() const
{
    std::vector<uint8_t> buffer;

    uint32_t size = static_cast<uint32_t>(bonus.size());
    buffer.insert(buffer.end(), reinterpret_cast<uint8_t *>(&size), reinterpret_cast<uint8_t *>(&size) + sizeof(size));

    for (const auto &entry : bonus)
    {
        TypeBonus type = std::get<0>(entry);
        int value = std::get<1>(entry);

        buffer.insert(buffer.end(), reinterpret_cast<uint8_t *>(&type),
                      reinterpret_cast<uint8_t *>(&type) + sizeof(type));

        buffer.insert(buffer.end(), reinterpret_cast<uint8_t *>(&value),
                      reinterpret_cast<uint8_t *>(&value) + sizeof(value));
    }

    return buffer;
}

BonusComponent BonusComponent::deserialize(const uint8_t *data, size_t size)
{
    if (size < sizeof(uint32_t))
        throw std::runtime_error("Invalid data size for BonusComponent::deserialize");

    size_t offset = 0;

    uint32_t count;
    std::memcpy(&count, data + offset, sizeof(count));
    offset += sizeof(count);

    std::vector<std::tuple<TypeBonus, int>> bonusList;

    for (uint32_t i = 0; i < count; ++i)
    {
        if (offset + sizeof(TypeBonus) + sizeof(int) > size)
            throw std::runtime_error("Corrupted data in BonusComponent::deserialize");

        TypeBonus type;
        int value;

        std::memcpy(&type, data + offset, sizeof(type));
        offset += sizeof(type);

        std::memcpy(&value, data + offset, sizeof(value));
        offset += sizeof(value);

        bonusList.emplace_back(type, value);
    }

    return BonusComponent(bonusList);
}
