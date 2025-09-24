#pragma once

#include "using.hpp"
#include <iostream>
#include <stdint.h>

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
