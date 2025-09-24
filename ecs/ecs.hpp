#pragma once
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

// Forward declarations
class Component;
class Entity;
class EntityManager;

// Forward declarations des composants
class TransformComponent;
class PlayerComponent;
class VelocityComponent;
class LaserWarningComponent;
class CenteredComponent;
class SpriteComponent;
class ColliderComponent;
class HealthComponent;
class InputComponent;
class ProjectileComponent;
class EnemyComponent;
class BirdComponent;
class PipeComponent;
class GravityComponent;
class JumpComponent;
class GameStateComponent;

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
        std::cout << "Component " << typeid(T).name() << " assigned ID: " << static_cast<int>(typeID) << std::endl;
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

// Entity class
class Entity
{
  private:
    EntityID id;
    EntityManager &manager;
    bool active = true;
    std::vector<std::unique_ptr<Component>> components;
    ComponentMask componentMask;

    std::vector<uint8_t> serializeComponent(ComponentID compId) const
    {
        if (compId < components.size() && components[compId])
        {
            return components[compId]->serialize();
        }
        return {};
    }

    void deserializeComponent(ComponentID compId, const uint8_t *data, size_t dataSize)
    {
        (void)dataSize; // dataSize n'est pas utilisé car les méthodes deserialize connaissent leur taille

        if (compId == getComponentTypeID<TransformComponent>())
        {
            if (hasComponent<TransformComponent>())
                getComponent<TransformComponent>() = TransformComponent::deserialize(data);
            else
                addComponent<TransformComponent>(TransformComponent::deserialize(data));
        }
        else if (compId == getComponentTypeID<PlayerComponent>())
        {
            if (hasComponent<PlayerComponent>())
                getComponent<PlayerComponent>() = PlayerComponent::deserialize(data);
            else
                addComponent<PlayerComponent>(PlayerComponent::deserialize(data));
        }
        else if (compId == getComponentTypeID<VelocityComponent>())
        {
            if (hasComponent<VelocityComponent>())
                getComponent<VelocityComponent>() = VelocityComponent::deserialize(data);
            else
                addComponent<VelocityComponent>(VelocityComponent::deserialize(data));
        }
        else if (compId == getComponentTypeID<LaserWarningComponent>())
        {
            if (hasComponent<LaserWarningComponent>())
                getComponent<LaserWarningComponent>() = LaserWarningComponent::deserialize(data);
            else
                addComponent<LaserWarningComponent>(LaserWarningComponent::deserialize(data));
        }
        else if (compId == getComponentTypeID<CenteredComponent>())
        {
            if (hasComponent<CenteredComponent>())
                getComponent<CenteredComponent>() = CenteredComponent::deserialize(data);
            else
                addComponent<CenteredComponent>(CenteredComponent::deserialize(data));
        }
        else if (compId == getComponentTypeID<SpriteComponent>())
        {
            if (hasComponent<SpriteComponent>())
                getComponent<SpriteComponent>() = SpriteComponent::deserialize(data);
            else
                addComponent<SpriteComponent>(SpriteComponent::deserialize(data));
        }
        else if (compId == getComponentTypeID<ColliderComponent>())
        {
            if (hasComponent<ColliderComponent>())
                getComponent<ColliderComponent>() = ColliderComponent::deserialize(data);
            else
                addComponent<ColliderComponent>(ColliderComponent::deserialize(data));
        }
        else if (compId == getComponentTypeID<HealthComponent>())
        {
            if (hasComponent<HealthComponent>())
                getComponent<HealthComponent>() = HealthComponent::deserialize(data);
            else
                addComponent<HealthComponent>(HealthComponent::deserialize(data));
        }
        else if (compId == getComponentTypeID<InputComponent>())
        {
            if (hasComponent<InputComponent>())
                getComponent<InputComponent>() = InputComponent::deserialize(data);
            else
                addComponent<InputComponent>(InputComponent::deserialize(data));
        }
        else if (compId == getComponentTypeID<ProjectileComponent>())
        {
            if (hasComponent<ProjectileComponent>())
                getComponent<ProjectileComponent>() = ProjectileComponent::deserialize(data);
            else
                addComponent<ProjectileComponent>(ProjectileComponent::deserialize(data));
        }
        else if (compId == getComponentTypeID<EnemyComponent>())
        {
            if (hasComponent<EnemyComponent>())
                getComponent<EnemyComponent>() = EnemyComponent::deserialize(data);
            else
                addComponent<EnemyComponent>(EnemyComponent::deserialize(data));
        }
        else if (compId == getComponentTypeID<BirdComponent>())
        {
            if (hasComponent<BirdComponent>())
                getComponent<BirdComponent>() = BirdComponent::deserialize(data);
            else
                addComponent<BirdComponent>(BirdComponent::deserialize(data));
        }
        else if (compId == getComponentTypeID<PipeComponent>())
        {
            if (hasComponent<PipeComponent>())
                getComponent<PipeComponent>() = PipeComponent::deserialize(data);
            else
                addComponent<PipeComponent>(PipeComponent::deserialize(data));
        }
        else if (compId == getComponentTypeID<GravityComponent>())
        {
            if (hasComponent<GravityComponent>())
                getComponent<GravityComponent>() = GravityComponent::deserialize(data);
            else
                addComponent<GravityComponent>(GravityComponent::deserialize(data));
        }
        else if (compId == getComponentTypeID<JumpComponent>())
        {
            if (hasComponent<JumpComponent>())
                getComponent<JumpComponent>() = JumpComponent::deserialize(data);
            else
                addComponent<JumpComponent>(JumpComponent::deserialize(data));
        }
        else if (compId == getComponentTypeID<GameStateComponent>())
        {
            if (hasComponent<GameStateComponent>())
                getComponent<GameStateComponent>() = GameStateComponent::deserialize(data);
            else
                addComponent<GameStateComponent>(GameStateComponent::deserialize(data));
        }
        // else: ID de composant inconnu, peut-être enregistrer une erreur
    }

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

    std::vector<uint8_t> serialize() const
    {
        std::vector<uint8_t> data;

        // Écrire EntityID
        data.insert(data.end(), reinterpret_cast<const uint8_t *>(&id),
                    reinterpret_cast<const uint8_t *>(&id) + sizeof(EntityID));

        // Écrire ComponentMask
        uint32_t maskValue = static_cast<uint32_t>(componentMask.to_ulong());
        data.insert(data.end(), reinterpret_cast<const uint8_t *>(&maskValue),
                    reinterpret_cast<const uint8_t *>(&maskValue) + sizeof(uint32_t));

        // Compter les composants actifs
        uint8_t componentCount = 0;
        for (ComponentID i = 0; i < MAX_COMPONENTS; ++i)
        {
            if (componentMask[i] && i < components.size() && components[i])
            {
                componentCount++;
            }
        }
        data.push_back(componentCount);

        // Sérialiser chaque composant
        for (ComponentID i = 0; i < MAX_COMPONENTS; ++i)
        {
            if (componentMask[i] && i < components.size() && components[i])
            {
                data.push_back(i); // ComponentID

                // Obtenir les données sérialisées du composant
                auto componentData = serializeComponent(i);

                // Écrire la taille des données du composant
                uint16_t dataSize = static_cast<uint16_t>(componentData.size());
                data.insert(data.end(), reinterpret_cast<const uint8_t *>(&dataSize),
                            reinterpret_cast<const uint8_t *>(&dataSize) + sizeof(uint16_t));

                // Écrire les données du composant
                data.insert(data.end(), componentData.begin(), componentData.end());
            }
        }

        return data;
    }

    size_t deserialize(const uint8_t *data, size_t maxSize)
    {
        size_t offset = 0;

        if (offset + sizeof(EntityID) > maxSize)
            return offset;

        // Lire EntityID (mais ne pas le changer - il est défini par EntityManager)
        EntityID serializedId;
        std::memcpy(&serializedId, data + offset, sizeof(EntityID));
        offset += sizeof(EntityID);

        if (offset + sizeof(uint32_t) > maxSize)
            return offset;

        // Lire ComponentMask
        uint32_t maskValue;
        std::memcpy(&maskValue, data + offset, sizeof(uint32_t));
        offset += sizeof(uint32_t);
        componentMask = ComponentMask(maskValue);

        if (offset + sizeof(uint8_t) > maxSize)
            return offset;

        // Lire le nombre de composants
        uint8_t componentCount = data[offset++];

        // Désérialiser chaque composant
        for (uint8_t i = 0; i < componentCount; ++i)
        {
            if (offset + sizeof(ComponentID) > maxSize)
                break;
            ComponentID compId = data[offset++];

            if (offset + sizeof(uint16_t) > maxSize)
                break;
            uint16_t dataSize;
            std::memcpy(&dataSize, data + offset, sizeof(uint16_t));
            offset += sizeof(uint16_t);

            if (offset + dataSize > maxSize)
                break;

            // Désérialiser le composant basé sur l'ID
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
            if (c)
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

        // S'assurer que le vecteur components est assez grand
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
            if (e && e->isActive())
            {
                e->update(deltaTime);
            }
        }
    }

    std::vector<uint8_t> serializeAllEntities() const
    {
        std::vector<uint8_t> data;

        // Écrire le nombre d'entités actives
        uint32_t activeEntityCount = 0;
        for (const auto &entity : entities)
        {
            if (entity && entity->isActive())
            {
                activeEntityCount++;
            }
        }

        data.insert(data.end(), reinterpret_cast<const uint8_t *>(&activeEntityCount),
                    reinterpret_cast<const uint8_t *>(&activeEntityCount) + sizeof(uint32_t));

        // Sérialiser chaque entité active
        for (const auto &entity : entities)
        {
            if (entity && entity->isActive())
            {
                auto entityData = entity->serialize();
                data.insert(data.end(), entityData.begin(), entityData.end());
            }
        }

        return data;
    }

    // Désérialiser les entités à partir du vecteur d'octets
    void deserializeAllEntities(const std::vector<uint8_t> &data)
    {
        if (data.size() < sizeof(uint32_t))
            return;

        size_t offset = 0;

        // Lire le nombre d'entités
        uint32_t entityCount;
        std::memcpy(&entityCount, data.data() + offset, sizeof(uint32_t));
        offset += sizeof(uint32_t);

        // Effacer les entités existantes
        entities.clear();

        // Désérialiser chaque entité
        for (uint32_t i = 0; i < entityCount && offset < data.size(); ++i)
        {
            auto &newEntity = createEntity();
            size_t bytesRead = newEntity.deserialize(data.data() + offset, data.size() - offset);
            offset += bytesRead;
        }

        refresh();
    }

    void render()
    {
        for (auto &e : entities)
        {
            if (e && e->isActive())
            {
                e->render();
            }
        }
    }

    void refresh()
    {
        // Supprimer les entités inactives
        entities.erase(
            std::remove_if(entities.begin(), entities.end(),
                           [](const std::unique_ptr<Entity> &entity) { return !entity || !entity->isActive(); }),
            entities.end());

        // Mettre à jour les listes d'entités par composant
        for (auto &componentEntities : entitiesByComponent)
        {
            componentEntities.clear();
        }

        for (auto &entity : entities)
        {
            if (entity)
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
                if (entity && (entity->getComponentMask() & mask) == mask)
                {
                    matchingEntities.push_back(entity.get());
                }
            }
        }
        return matchingEntities;
    }

    // Méthodes utilitaires pour la gestion des entités
    size_t getEntityCount() const
    {
        return entities.size();
    }

    size_t getActiveEntityCount() const
    {
        size_t count = 0;
        for (const auto &entity : entities)
        {
            if (entity && entity->isActive())
            {
                count++;
            }
        }
        return count;
    }

    void clear()
    {
        entities.clear();
        for (auto &componentEntities : entitiesByComponent)
        {
            componentEntities.clear();
        }
    }
};