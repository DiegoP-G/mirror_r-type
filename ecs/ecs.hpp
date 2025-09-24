#pragma once
#include <algorithm>
#include <array>
#include <bitset>
#include <cstring>
#include <iostream>
#include <memory>
#include <set>
#include <unordered_map>
#include <vector>

// Forward declarations
class Component;
class Entity;
class EntityManager;

// Types
using EntityID = int32_t;
using ComponentID = int8_t;

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
        std::cout << "Component " << typeid(T).name() << " assigned ID: " << typeID << std::endl;
        printed = true;
    }
    return typeID;
}
// Base Component class
class Component
{
  public:
    Entity *entity;

    virtual ~Component() = default;
    virtual void init()
    {
    }
    virtual void update(float deltaTime)
    {
    }
    virtual void render()
    {
    }
};
// Entity class
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
            c->update(deltaTime);
    }

    std::vector<uint8_t> serialize() const
    {
        std::vector<uint8_t> data;

        // Write EntityID
        data.insert(data.end(), reinterpret_cast<const uint8_t *>(&id),
                    reinterpret_cast<const uint8_t *>(&id) + sizeof(EntityID));

        // Write ComponentMask
        uint32_t maskValue = static_cast<uint32_t>(componentMask.to_ulong());
        data.insert(data.end(), reinterpret_cast<const uint8_t *>(&maskValue),
                    reinterpret_cast<const uint8_t *>(&maskValue) + sizeof(uint32_t));

        // Count active components
        uint8_t componentCount = 0;
        for (ComponentID i = 0; i < MAX_COMPONENTS; ++i)
        {
            if (componentMask[i] && i < components.size() && components[i])
            {
                componentCount++;
            }
        }
        data.push_back(componentCount);

        // Serialize each component
        for (ComponentID i = 0; i < MAX_COMPONENTS; ++i)
        {
            if (componentMask[i] && i < components.size() && components[i])
            {
                data.push_back(i); // ComponentID

                // Get component serialized data
                auto componentData = serializeComponent(i);

                // Write component data size
                uint16_t dataSize = static_cast<uint16_t>(componentData.size());
                data.insert(data.end(), reinterpret_cast<const uint8_t *>(&dataSize),
                            reinterpret_cast<const uint8_t *>(&dataSize) + sizeof(uint16_t));

                // Write component data
                data.insert(data.end(), componentData.begin(), componentData.end());
            }
        }

        return data;
    }

    size_t deserialize(const uint8_t *data, size_t maxSize)
    {
        size_t offset = 0;

        // Read EntityID (but don't change it - it's set by EntityManager)
        EntityID serializedId;
        std::memcpy(&serializedId, data + offset, sizeof(EntityID));
        offset += sizeof(EntityID);

        // Read ComponentMask
        uint32_t maskValue;
        std::memcpy(&maskValue, data + offset, sizeof(uint32_t));
        offset += sizeof(uint32_t);
        componentMask = ComponentMask(maskValue);

        // Read component count
        uint8_t componentCount = data[offset++];

        // Deserialize each component
        for (uint8_t i = 0; i < componentCount; ++i)
        {
            ComponentID compId = data[offset++];

            uint16_t dataSize;
            std::memcpy(&dataSize, data + offset, sizeof(uint16_t));
            offset += sizeof(uint16_t);

            // Deserialize component based on ID
            deserializeComponent(compId, data + offset, dataSize);
            offset += dataSize;
        }

        return offset;
    }

    bool isActive() const
    {
        return active;
    }

    void render()
    {
        for (auto &c : components)
            c->render();
    }

    void destroy()
    {
        active = false;
    }

    EntityID getID() const
    {
        return id;
    }

    template <typename T, typename... TArgs> T &addComponent(TArgs &&...args)
    {
        ComponentID componentID = getComponentTypeID<T>();

        // Ensure components vector is large enough
        if (components.size() <= componentID)
        {
            components.resize(componentID + 1);
        }

        T *c = new T(std::forward<TArgs>(args)...);
        c->entity = this;

        std::unique_ptr<Component> uPtr(c);
        components[componentID] = std::move(uPtr);
        componentMask.set(componentID);

        c->init();
        return *c;
    }

    template <typename T> bool hasComponent() const
    {
        return componentMask[getComponentTypeID<T>()];
    }

    template <typename T> T &getComponent()
    {
        ComponentID componentID = getComponentTypeID<T>();

        // Check if component exists first
        if (!componentMask[componentID])
        {
            throw std::runtime_error("Entity miss this component : " + std::to_string(componentID));
        }

        // Ensure the components vector is large enough
        if (componentID >= components.size() || !components[componentID])
        {
            throw std::runtime_error("Component not found in storage!");
        }

        // Direct access by component ID instead of linear search
        return *static_cast<T *>(components[componentID].get());
    }

    ComponentMask getComponentMask() const
    {
        return componentMask;
    }
};

// Entity Manager
class EntityManager
{
  private:
    std::vector<std::unique_ptr<Entity>> entities;
    std::array<std::vector<Entity *>, MAX_COMPONENTS> entitiesByComponent;

  public:
    void update(float deltaTime)
    {
        for (auto &e : entities)
        {
            if (e->isActive())
            {
                e->update(deltaTime);
            }
        }
    }

    std::vector<uint8_t> serializeAllEntities() const
    {
        std::vector<uint8_t> data;

        // Write number of active entities
        uint32_t activeEntityCount = 0;
        for (const auto &entity : entities)
        {
            if (entity->isActive())
            {
                activeEntityCount++;
            }
        }

        data.insert(data.end(), reinterpret_cast<const uint8_t *>(&activeEntityCount),
                    reinterpret_cast<const uint8_t *>(&activeEntityCount) + sizeof(uint32_t));

        // Serialize each active entity
        for (const auto &entity : entities)
        {
            if (entity->isActive())
            {
                auto entityData = entity->serialize();
                data.insert(data.end(), entityData.begin(), entityData.end());
            }
        }

        return data;
    }

    // Deserialize entities from byte vector
    void deserializeAllEntities(const std::vector<uint8_t> &data)
    {
        size_t offset = 0;

        // Read number of entities
        uint32_t entityCount;
        std::memcpy(&entityCount, data.data() + offset, sizeof(uint32_t));
        offset += sizeof(uint32_t);

        // Clear existing entities
        entities.clear();

        // Deserialize each entity
        for (uint32_t i = 0; i < entityCount; ++i)
        {
            auto &newEntity = createEntity();
            offset += newEntity.deserialize(data.data() + offset, data.size() - offset);
        }

        refresh();
    }

    void render()
    {
        for (auto &e : entities)
        {
            if (e->isActive())
            {
                e->render();
            }
        }
    }

    void refresh()
    {
        // Remove inactive entities
        entities.erase(std::remove_if(entities.begin(), entities.end(),
                                      [](const std::unique_ptr<Entity> &entity) { return !entity->isActive(); }),
                       entities.end());

        // Update component entity lists
        for (auto &componentEntities : entitiesByComponent)
        {
            componentEntities.clear();
        }

        for (auto &entity : entities)
        {
            for (ComponentID i = 0; i < MAX_COMPONENTS; i++)
            {
                if (entity->getComponentMask().test(i))
                {
                    entitiesByComponent[i].push_back(entity.get());
                }
            }
        }
    }

    Entity &createEntity()
    {
        EntityID id = static_cast<EntityID>(entities.size());
        Entity *e = new Entity(*this, id);
        entities.emplace_back(e);
        return *e;
    }

    template <typename T> std::vector<Entity *> &getEntitiesWithComponent()
    {
        return entitiesByComponent[getComponentTypeID<T>()];
    }

    template <typename... Ts> std::vector<Entity *> getEntitiesWithComponents()
    {
        std::vector<Entity *> matchingEntities;

        if constexpr (sizeof...(Ts) > 0)
        {
            ComponentMask mask;
            (mask.set(getComponentTypeID<Ts>()), ...);

            for (auto &entity : entities)
            {
                if ((entity->getComponentMask() & mask) == mask)
                {
                    matchingEntities.push_back(entity.get());
                }
            }
        }
        return matchingEntities;
    }
};