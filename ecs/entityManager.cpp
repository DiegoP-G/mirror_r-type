#include "entityManager.hpp"
#include <algorithm>

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

    void EntityManager::clear()
    {
        entities.clear();
        for (auto &componentEntities : entitiesByComponent)
        {
            componentEntities.clear();
        }
    }
