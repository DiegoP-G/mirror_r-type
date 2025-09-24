#pragma once

#include "components.hpp"
#include "using.hpp"
#include <cstdint>
#include <cstring>
#include <memory>
#include <vector>

class EntityManager;

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
    Entity(EntityManager &manager, EntityID id);

    void update(float deltaTime);

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