#pragma once
#include <stdint.h>
#include <vector>

class Entity;

// Base Component class
class IComponent
{
  public:
    Entity *entity;

    virtual ~IComponent() = default;

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

    virtual bool operator==(const IComponent &other) const
    {
        return false;
    }
};
