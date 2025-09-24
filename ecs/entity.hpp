#pragma once

#include "components.hpp"
#include "using.hpp"
#include "ecs.hpp"
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

    // Ajoute un composant de type T à l'entité. Le remplace s'il existe déjà.
    template <typename T, typename... TArgs> T &addComponent(TArgs &&...args)
        {
        ComponentID componentID = getComponentTypeID<T>();

        // S'assurer que le vecteur components est assez grand
        if (components.size() <= componentID)
        {
            components.resize(componentID + 1);
        }

        T *c = new T(std::forward<TArgs>(args)...);
        c->entity = this;

        components[componentID].reset(c);
        componentMask.set(componentID);

        c->init();
        return *c;
    };

    template <typename T> bool hasComponent() const
        {
        ComponentID componentID = getComponentTypeID<T>();
        return componentID < componentMask.size() && componentMask[componentID];
    }

    template <typename T> T &getComponent()
        {
        ComponentID componentID = getComponentTypeID<T>();

        // Vérifier si le composant existe d'abord
        if (componentID >= componentMask.size() || !componentMask[componentID])
        {
            throw std::runtime_error("Entity miss this component : " + std::to_string(componentID));
        }

        // S'assurer que le vecteur components est assez grand
        if (componentID >= components.size() || !components[componentID])
        {
            throw std::runtime_error("Component not found in storage!");
        }

        // Accès direct par ID de composant au lieu de recherche linéaire
        return *static_cast<T *>(components[componentID].get());
    }

    ComponentMask getComponentMask() const;
};