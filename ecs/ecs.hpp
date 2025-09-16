#pragma once
#include <iostream>
#include <vector>
#include <memory>
#include <unordered_map>
#include <bitset>
#include <array>
#include <set>
#include <algorithm>

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
inline ComponentID getNextComponentID() {
    static ComponentID nextID = 0;
    return nextID++;
}

// Component ID getter for a specific type
template <typename T>
inline ComponentID getComponentTypeID() {
    static ComponentID typeID = getNextComponentID();
    return typeID;
}
// Base Component class
class Component {
public:
    Entity* entity;
    
    virtual ~Component() = default;
    virtual void init() {}
    virtual void update(float deltaTime) {}
    virtual void render() {}
};
// Entity class
class Entity {
private:
EntityID id;
    EntityManager& manager;
    bool active = true;
    std::vector<std::unique_ptr<Component>> components;
    ComponentMask componentMask;
    
    public:
    Entity(EntityManager& manager, EntityID id) : manager(manager), id(id) {}
    
    void update(float deltaTime) {
        for (auto& c : components) c->update(deltaTime);
    }
    
    void render() {
        for (auto& c : components) c->render();
    }
    
    bool isActive() const { return active; }
    void destroy() { active = false; }
    
    EntityID getID() const { return id; }
    
    template <typename T, typename... TArgs>
    T& addComponent(TArgs&&... args) {
        ComponentID componentID = getComponentTypeID<T>();
        
        // Ensure components vector is large enough
        if (components.size() <= componentID) {
            components.resize(componentID + 1);
        }
        
        T* c = new T(std::forward<TArgs>(args)...);
        c->entity = this;
        
        std::unique_ptr<Component> uPtr(c);
        components[componentID] = std::move(uPtr);
        componentMask.set(componentID);
        
        c->init();
        return *c;
    }
    
    template <typename T>
    bool hasComponent() const {
        return componentMask[getComponentTypeID<T>()];
    }
    
    template <typename T>
    T& getComponent() {
        ComponentID componentID = getComponentTypeID<T>();
        
        // Check if component exists first
        if (!componentMask[componentID]) {
            throw std::runtime_error("Entity does not have component!");
        }
        
        // Ensure the components vector is large enough
        if (componentID >= components.size() || !components[componentID]) {
            throw std::runtime_error("Component not found in storage!");
        }
        
        // Direct access by component ID instead of linear search
        return *static_cast<T*>(components[componentID].get());
    }
    
    ComponentMask getComponentMask() const { return componentMask; }
};



// Entity Manager
class EntityManager {
    private:
    std::vector<std::unique_ptr<Entity>> entities;
    std::array<std::vector<Entity*>, MAX_COMPONENTS> entitiesByComponent;
    
    public:
    void update(float deltaTime) {
        for (auto& e : entities) {
            if (e->isActive()) {
                e->update(deltaTime);
            }
        }
    }
    
    void render() {
        for (auto& e : entities) {
            if (e->isActive()) {
                e->render();
            }
        }
    }
    
    void refresh() {
        // Remove inactive entities
        entities.erase(
            std::remove_if(entities.begin(), entities.end(),
                [](const std::unique_ptr<Entity>& entity) {
                    return !entity->isActive();
                }),
            entities.end());
            
        // Update component entity lists
        for (auto& componentEntities : entitiesByComponent) {
            componentEntities.clear();
        }
        
        for (auto& entity : entities) {
            for (ComponentID i = 0; i < MAX_COMPONENTS; i++) {
                if (entity->getComponentMask().test(i)) {
                    entitiesByComponent[i].push_back(entity.get());
                }
            }
        }
    }
    
    Entity& createEntity() {
        EntityID id = static_cast<EntityID>(entities.size());
        Entity* e = new Entity(*this, id);
        entities.emplace_back(e);
        return *e;
    }
    
    template <typename T>
    std::vector<Entity*>& getEntitiesWithComponent() {
        return entitiesByComponent[getComponentTypeID<T>()];
    }
    
    template <typename... Ts>
    std::vector<Entity*> getEntitiesWithComponents() {
        std::vector<Entity*> matchingEntities;
        
        if constexpr (sizeof...(Ts) > 0) {
            ComponentMask mask;
            (mask.set(getComponentTypeID<Ts>()), ...);
            
            for (auto& entity : entities) {
                if ((entity->getComponentMask() & mask) == mask) {
                    matchingEntities.push_back(entity.get());
                }
            }
        }
        
        return matchingEntities;
    }
};