#pragma once
#include <vector>
#include <array>
#include <memory>
#include "entity.hpp"
#include "ecs.hpp"
// Entity Manager
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

    template <typename T> std::vector<Entity *> &getEntitiesWithComponent();

    template <typename... Ts> std::vector<Entity *> getEntitiesWithComponents();

    size_t getEntityCount() const;

    size_t getActiveEntityCount() const;

    void clear();
};
