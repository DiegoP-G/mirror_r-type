#include "entityManager.hpp"
#include <algorithm>
#include "components/PlayerComponent.hpp"
#include "components/EnemyComponent.hpp"
#include "components/ProjectileComponent.hpp"

void EntityManager::update(float deltaTime)
{
    for (auto &e : entities)
    {
        if (e && e->isActive())
        {
            e->update(deltaTime);
        }
    }
}

std::vector<uint8_t> EntityManager::serializeAllPlayers() const
{
    std::vector<uint8_t> data;

    // Compter d'abord les joueurs actifs
    uint32_t playerCount = 0;
    for (const auto &entity : entities)
    {
        if (entity && entity->isActive() && entity->hasComponent<PlayerComponent>())
        {
            playerCount++;
        }
    }

    // Écrire le nombre de joueurs
    data.insert(data.end(), reinterpret_cast<const uint8_t *>(&playerCount),
                reinterpret_cast<const uint8_t *>(&playerCount) + sizeof(uint32_t));

    // Sérialiser chaque joueur
    for (const auto &entity : entities)
    {
        if (entity && entity->isActive() && entity->hasComponent<PlayerComponent>())
        {
            auto entityData = entity->serialize();
            data.insert(data.end(), entityData.begin(), entityData.end());
        }
    }

    return data;
}

std::vector<uint8_t> EntityManager::serializeAllEnemies() const
{
    std::vector<uint8_t> data;

    // Compter d'abord les ennemis actifs
    uint32_t enemyCount = 0;
    for (const auto &entity : entities)
    {
        if (entity && entity->isActive() && entity->hasComponent<EnemyComponent>())
        {
            enemyCount++;
        }
    }

    // Écrire le nombre d'ennemis
    data.insert(data.end(), reinterpret_cast<const uint8_t *>(&enemyCount),
                reinterpret_cast<const uint8_t *>(&enemyCount) + sizeof(uint32_t));

    // Sérialiser chaque ennemi
    for (const auto &entity : entities)
    {
        if (entity && entity->isActive() && entity->hasComponent<EnemyComponent>())
        {
            auto entityData = entity->serialize();
            data.insert(data.end(), entityData.begin(), entityData.end());
        }
    }

    return data;
}

std::vector<uint8_t> EntityManager::serializeAllProjectiles() const
{
    std::vector<uint8_t> data;

    // Compter d'abord les projectiles actifs
    uint32_t projectileCount = 0;
    for (const auto &entity : entities)
    {
        if (entity && entity->isActive() && entity->hasComponent<ProjectileComponent>())
        {
            projectileCount++;
        }
    }

    // Écrire le nombre de projectiles
    data.insert(data.end(), reinterpret_cast<const uint8_t *>(&projectileCount),
                reinterpret_cast<const uint8_t *>(&projectileCount) + sizeof(uint32_t));

    // Sérialiser chaque projectile
    for (const auto &entity : entities)
    {
        if (entity && entity->isActive() && entity->hasComponent<ProjectileComponent>())
        {
            auto entityData = entity->serialize();
            data.insert(data.end(), entityData.begin(), entityData.end());
        }
    }

    return data;
}


std::vector<uint8_t> EntityManager::serializeAllEntities() const
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

    // std::vector<uint8_t> data;

    // // Écrire le nombre d'entités actives
    // uint32_t activeEntityCount = 1;


    // data.insert(data.end(), reinterpret_cast<const uint8_t *>(&activeEntityCount),
    //             reinterpret_cast<const uint8_t *>(&activeEntityCount) + sizeof(uint32_t));

    // // Sérialiser chaque entité active
    // for (const auto &entity : entities)
    // {
    //     if (entity && entity->isActive() && entity->hasComponent<InputComponent>())
    //     {
    //         auto entityData = entity->serialize();
    //         data.insert(data.end(), entityData.begin(), entityData.end());
    //     }
    // }

    // return data;
}


void EntityManager::deserializePlayerEntities(const std::vector<uint8_t> &data)
{
    if (data.size() < sizeof(uint32_t))
        return;
    
    size_t offset = 0;

    // Lire le nombre de joueurs
    uint32_t playerCount;
    std::memcpy(&playerCount, data.data() + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    // Supprimer les joueurs existants
    auto playerEntities = getEntitiesWithComponent<PlayerComponent>();
    for (auto *entity : playerEntities)
    {
        entity->destroy();
    }
    refresh();

    // Désérialiser chaque joueur
    for (uint32_t i = 0; i < playerCount && offset < data.size(); ++i)
    {
        auto &newEntity = createEntity();
        size_t bytesRead = newEntity.deserialize(data.data() + offset, data.size() - offset);
        offset += bytesRead;
    }

    refresh();
}

void EntityManager::deserializeEnemyEntities(const std::vector<uint8_t> &data)
{
    if (data.size() < sizeof(uint32_t))
        return;
    
    size_t offset = 0;

    // Lire le nombre d'ennemis
    uint32_t enemyCount;
    std::memcpy(&enemyCount, data.data() + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    // Supprimer les ennemis existants
    auto enemyEntities = getEntitiesWithComponent<EnemyComponent>();
    for (auto *entity : enemyEntities)
    {
        entity->destroy();
    }
    refresh();

    // Désérialiser chaque ennemi
    for (uint32_t i = 0; i < enemyCount && offset < data.size(); ++i)
    {
        auto &newEntity = createEntity();
        size_t bytesRead = newEntity.deserialize(data.data() + offset, data.size() - offset);
        offset += bytesRead;
    }

    refresh();
}

void EntityManager::deserializeProjectileEntities(const std::vector<uint8_t> &data)
{
    if (data.size() < sizeof(uint32_t))
        return;
    
    size_t offset = 0;

    // Lire le nombre de projectiles
    uint32_t projectileCount;
    std::memcpy(&projectileCount, data.data() + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    // Supprimer les projectiles existants
    auto projectileEntities = getEntitiesWithComponent<ProjectileComponent>();
    for (auto *entity : projectileEntities)
    {
        entity->destroy();
    }
    refresh();

    // Désérialiser chaque projectile
    for (uint32_t i = 0; i < projectileCount && offset < data.size(); ++i)
    {
        auto &newEntity = createEntity();
        size_t bytesRead = newEntity.deserialize(data.data() + offset, data.size() - offset);
        offset += bytesRead;
    }

    refresh();
}



// Désérialiser les entités à partir du vecteur d'octets
void EntityManager::deserializeAllEntities(const std::vector<uint8_t> &data)
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

void EntityManager::render()
{
    for (auto &e : entities)
    {
        if (e && e->isActive())
        {
            e->render();
        }
    }
}

void EntityManager::refresh()
{
    // Supprimer les entités inactives
    entities.erase(std::remove_if(entities.begin(), entities.end(),
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

Entity &EntityManager::createEntity()
{
    EntityID id = static_cast<EntityID>(entities.size());
    Entity *e = new Entity(*this, id);
    entities.emplace_back(e);
    return *e;
}

// Méthodes utilitaires pour la gestion des entités
size_t EntityManager::getEntityCount() const
{
    return entities.size();
}

size_t EntityManager::getActiveEntityCount() const
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

Entity *EntityManager::getEntityByID(EntityID id)
{
    for (const auto &entity : entities)
    {
        if (entity && entity->getID() == id)
        {
            return entity.get();
        }
    }
    return nullptr;
}

void EntityManager::clear()
{
    entities.clear();
    for (auto &componentEntities : entitiesByComponent)
    {
        componentEntities.clear();
    }
}
