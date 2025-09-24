#pragma once
#include <bitset>
#include <stdint.h>

// Types
using ComponentID = int8_t;
using EntityID = int32_t;

// Maximum number of components and entities
constexpr std::size_t MAX_COMPONENTS = 32;
using ComponentMask = std::bitset<MAX_COMPONENTS>;