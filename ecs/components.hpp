#pragma once
#include <stdint.h>
#include <vector>

class Entity;

// Base Component class
class Component
{
  public:
    Entity *entity;

    virtual ~Component() = default;

    virtual void init()
    {
    }

    virtual std::vector<uint8_t> serialize() const
    {
        return {};
    }

    virtual void update(float deltaTime)
    {
    }

    virtual void render()
    {
    }
};
