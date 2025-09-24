#pragma once

#include <vector>
#include <memory>
#include <bitset>
#include <cstdint>
#include <cstring>
#include "ecs.hpp"
#include "entityManager.hpp"
#include "allComponentsInclude.hpp"

class Entity
{
  private:
    EntityID id;
    EntityManager &manager;
    bool active = true;
    std::vector<std::unique_ptr<Component>> components;
    ComponentMask componentMask;

    std::vector<uint8_t> serializeComponent(ComponentID compId) const;

    void deserializeComponent(ComponentID compId, const uint8_t *data, size_t dataSize);

  public:
    Entity(EntityManager &manager, EntityID id) : manager(manager), id(id)
    {
    }

    void update(float deltaTime)
    {
        for (auto &c : components)
            if (c)
                c->update(deltaTime);
    }

    std::vector<uint8_t> serialize() const;

    size_t deserialize(const uint8_t *data, size_t maxSize);

    bool isActive() const;

    void render();

    void destroy();

    EntityID getID() const;

    template <typename T, typename... TArgs> T &addComponent(TArgs &&...args);

    template <typename T> bool hasComponent() const;

    template <typename T> T &getComponent();

    ComponentMask getComponentMask() const;
};