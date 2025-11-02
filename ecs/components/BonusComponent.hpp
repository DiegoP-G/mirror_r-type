#pragma once
#include "../IComponent.hpp"

class BonusComponent : public IComponent
{

  public:
    enum TypeBonus
    {
        HEALTH,
        FIREMODE,
        SHIELD
    };
    BonusComponent(std::vector<std::tuple<TypeBonus, int>> bonus);
    void update(float deltaTime) override;
    std::vector<uint8_t> serialize() const override;
    static BonusComponent deserialize(const uint8_t *data, size_t size);
    std::vector<std::tuple<TypeBonus, int>> bonus;
};
