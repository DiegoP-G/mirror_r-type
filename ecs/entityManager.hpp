#pragma once
#include "entity.hpp"
#include <array>
#include <memory>
#include <mutex>
#include <vector>

class EntityManager
{
  private:
    std::vector<std::unique_ptr<Entity>> entities;
    std::array<std::vector<Entity *>, MAX_COMPONENTS> entitiesByComponent;

    // Buffers pour les opérations différées
    std::vector<EntityID> entitiesToDestroy;
    std::vector<std::unique_ptr<Entity>> entitiesToCreate;

  public:
    EntityManager();
    void update(float deltaTime);
    std::mutex entityMutex;

    void clear()
    {
        std::lock_guard<std::mutex> lock(entityMutex);
        entities.clear();
        for (auto &list : entitiesByComponent)
            list.clear();
        entitiesToDestroy.clear();
        entitiesToCreate.clear();
    }

    const std::vector<EntityID> &getEntitiesToDestroy() const
    {
        return entitiesToDestroy;
    }

    const std::vector<std::unique_ptr<Entity>> &getEntitiesToCreate() const
    {
        return entitiesToCreate;
    }

    // === SERVEUR SEULEMENT ===

    // Sérialisation complète d'une seule entité (TCP)
    std::vector<uint8_t> serializeEntityFull(EntityID id) const;

    // Sérialisation des mouvements de toutes les entités (UDP)
    std::vector<uint8_t> serializeAllMovements() const;
    std::vector<uint8_t> serializeAllHealth() const;

    // Buffer les entités à créer/détruire
    void markEntityForDestruction(EntityID id);
    Entity &queueEntityCreation();

    std::vector<Entity *> getPlayersDead(int &winnerID, bool &game_over);

    // Désérialisation complète d'une entité (TCP)
    void deserializeEntityFull(const std::vector<uint8_t> &data);

    // Désérialisation des mouvements (UDP)
    void deserializeAllMovements(const std::vector<uint8_t> &data);
    void deserializeAllHealth(const std::vector<uint8_t> &data);

    // Détruire une entité spécifique
    void destroyEntityByID(EntityID id);

    // === COMMUN ===

    void render();
    void refresh();
    void applyPendingChanges();

    Entity &createEntity(int newID = -1);

    size_t getEntityCount() const
    {
        return entities.size();
    }

    template <typename T> std::vector<Entity *> &getEntitiesWithComponent()
    {
        return entitiesByComponent[getComponentTypeID<T>()];
    };

    template <typename... Ts> std::vector<Entity *> getEntitiesWithComponents()
    {
        std::vector<Entity *> matchingEntities;

        if constexpr (sizeof...(Ts) > 0)
        {
            ComponentMask mask;
            (mask.set(getComponentTypeID<Ts>()), ...);

            for (auto &entity : entities)
            {
                if (entity && entity->isActive() && (entity->getComponentMask() & mask) == mask)
                {
                    matchingEntities.push_back(entity.get());
                }
            }
        }
        return matchingEntities;
    }

    template <typename... Ts> std::vector<Entity *> getInactiveEntitiesWithComponents()
    {
        std::vector<Entity *> matchingEntities;

        if constexpr (sizeof...(Ts) > 0)
        {
            ComponentMask mask;
            (mask.set(getComponentTypeID<Ts>()), ...);

            for (auto &entity : entities)
            {
                if (entity && !entity->isActive() && (entity->getComponentMask() & mask) == mask)
                {
                    matchingEntities.push_back(entity.get());
                }
            }
        }
        return matchingEntities;
    }

    template <typename... Ts> std::vector<Entity *> getAllEntitiesWithComponents()
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

    template <typename... Ts> int getCountEntitiesWithComponents()
    {
        std::vector<Entity *> matchingEntities;

        if constexpr (sizeof...(Ts) > 0)
        {
            ComponentMask mask;
            (mask.set(getComponentTypeID<Ts>()), ...);

            for (auto &entity : entities)
            {
                if (entity && entity->isActive() && (entity->getComponentMask() & mask) == mask)
                {
                    matchingEntities.push_back(entity.get());
                }
            }
        }
        return matchingEntities.size();
    }

    Entity *getEntityByID(EntityID id);
    size_t getActiveEntityCount() const;

    std::vector<std::unique_ptr<Entity>> &getEntities()
    {
        return entities;
    }

    std::vector<uint8_t> serializePlayersScores(const std::vector<std::pair<int, int>> &playersScores);
    std::vector<std::pair<int, int>> deserializePlayersScores(const std::vector<uint8_t> &data);
};