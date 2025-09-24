#pragma once
#include "entity.hpp"
#include "components.hpp"
#include <algorithm>
#include <array>
#include <bitset>
#include <cstring>
#include <iostream>
#include <memory>
#include <set>
#include <stdint.h>
#include <unordered_map>
#include <vector>


// Forward declarations des composants


// Types
using ComponentID = int8_t;
using EntityID = int32_t;

// Maximum number of components and entities
constexpr std::size_t MAX_COMPONENTS = 32;
using ComponentMask = std::bitset<MAX_COMPONENTS>;

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
        std::cout << "Component " << typeid(T).name() << " assigned ID: " << static_cast<int>(typeID) << std::endl;
        printed = true;
    }
    return typeID;
}
