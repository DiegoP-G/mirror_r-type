#pragma once
#include "entity.hpp"
#include <array>
#include <memory>
#include <vector>

class EntityManager
{
  private:
    std::vector<std::unique_ptr<Entity>> entities;
    std::array<std::vector<Entity *>, MAX_COMPONENTS> entitiesByComponent;

  public:
    void update(float deltaTime);

    std::vector<uint8_t> serializeAllEntities() const;

    // Désérialiser les entités à partir du vecteur d'octets
    void deserializeAllEntities(const std::vector<uint8_t> &data);

    void render();

    void refresh();

    Entity &createEntity();

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
                if (entity && (entity->getComponentMask() & mask) == mask)
                {
                    matchingEntities.push_back(entity.get());
                }
            }
        }
        return matchingEntities;
    }


    size_t getEntityCount() const;

    size_t getActiveEntityCount() const;

    void clear();
};
