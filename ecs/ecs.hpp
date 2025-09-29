#pragma once

#include "using.hpp"
#include <iostream>
#include <stdint.h>
#include "components/InputComponent.hpp"

// Component ID generator
inline ComponentID getNextComponentID()
{
    static ComponentID nextID = 0;
    return nextID++;
}

// Component ID getter for a specific type
template <typename T> inline ComponentID getComponentTypeID()
{
    static ComponentID typeID = getNextComponentID();
    static bool printed = false;
    if (!printed)
    {
        // std::cout << "Component " << typeid(T).name() << " assigned ID: " << static_cast<int>(typeID) << std::endl;
        printed = true;
    }
    return typeID;
}
const std::string &serializePlayerInput(const InputComponent &player, int playerId);
int deserializePlayerInput(const std::string &data, InputComponent &input);
