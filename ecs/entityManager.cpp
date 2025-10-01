#include "entityManager.hpp"
#include "components/EnemyComponent.hpp"
#include "components/PlayerComponent.hpp"
#include "components/ProjectileComponent.hpp"
#include <algorithm>

#include "ComponentFactory.hpp"
#include "components/AnimatedSpriteComponent.hpp"
#include "components/BackgroundScrollComponent.hpp"
#include "components/CenteredComponent.hpp"
#include "components/ColliderComponent.hpp"
#include "components/EnemyComponent.hpp"
#include "components/GameStateComponent.hpp"
#include "components/GravityComponent.hpp"
#include "components/HealthComponent.hpp"
#include "components/JumpComponent.hpp"
#include "components/LaserWarningComponent.hpp"
#include "components/PipeComponent.hpp"
#include "components/PlayerComponent.hpp"
#include "components/ProjectileComponent.hpp"
#include "components/SpriteComponent.hpp"
#include "components/TransformComponent.hpp"
#include "components/VelocityComponent.hpp"

EntityManager::EntityManager()
{
    ComponentFactory::registerComponent<TransformComponent>();
    ComponentFactory::registerComponent<PlayerComponent>();
    ComponentFactory::registerComponent<VelocityComponent>();
    ComponentFactory::registerComponent<LaserWarningComponent>();
    ComponentFactory::registerComponent<CenteredComponent>();
    ComponentFactory::registerComponent<SpriteComponent>();
    ComponentFactory::registerComponent<ColliderComponent>();
    ComponentFactory::registerComponent<HealthComponent>();
    ComponentFactory::registerComponent<InputComponent>();
    ComponentFactory::registerComponent<ProjectileComponent>();
    ComponentFactory::registerComponent<EnemyComponent>();
    ComponentFactory::registerComponent<PipeComponent>();
    ComponentFactory::registerComponent<GravityComponent>();
    ComponentFactory::registerComponent<JumpComponent>();
    ComponentFactory::registerComponent<GameStateComponent>();
    ComponentFactory::registerComponent<AnimatedSpriteComponent>();
    ComponentFactory::registerComponent<BackgroundScrollComponent>();
}

// === SERVEUR - Sérialisation complète d'une entité (TCP) ===
std::vector<uint8_t> EntityManager::serializeEntityFull(EntityID id) const
{
    for (const auto &entity : entities)
    {
        if (entity && entity->isActive() && entity->getID() == id)
        {
            return entity->serialize(); // Tous les composants
        }
    }
    return {};
}

// === SERVEUR - Sérialisation des mouvements (UDP) ===
std::vector<uint8_t> EntityManager::serializeAllMovements() const
{
    std::vector<uint8_t> data;

    // Compter les entités actives
    uint32_t entityCount = 0;
    for (const auto &entity : entities)
    {
        if (entity && entity->isActive())
            entityCount++;
    }

    // Écrire le nombre d'entités
    data.insert(data.end(), reinterpret_cast<const uint8_t *>(&entityCount),
                reinterpret_cast<const uint8_t *>(&entityCount) + sizeof(uint32_t));

    // Pour chaque entité active
    for (const auto &entity : entities)
    {
        if (!entity || !entity->isActive())
            continue;

        // EntityID
        EntityID id = entity->getID();
        data.insert(data.end(), reinterpret_cast<const uint8_t *>(&id),
                    reinterpret_cast<const uint8_t *>(&id) + sizeof(EntityID));

        // Transform (position uniquement)
        if (entity->hasComponent<TransformComponent>())
        {
            auto &transform = entity->getComponent<TransformComponent>();
            auto posData = transform.position.serialize();
            data.insert(data.end(), posData.begin(), posData.end());
        }
        else
        {
            // Position par défaut si pas de transform
            Vector2D zero(0, 0);
            auto posData = zero.serialize();
            data.insert(data.end(), posData.begin(), posData.end());
        }

        // Velocity (optionnel)
        if (entity->hasComponent<VelocityComponent>())
        {
            auto &velocity = entity->getComponent<VelocityComponent>();
            auto velData = velocity.velocity.serialize();
            data.insert(data.end(), velData.begin(), velData.end());
        }
        else
        {
            // Velocity par défaut
            Vector2D zero(0, 0);
            auto velData = zero.serialize();
            data.insert(data.end(), velData.begin(), velData.end());
        }
    }

    return data;
}

Entity &EntityManager::createEntity()
{
    EntityID id = static_cast<EntityID>(entities.size());
    auto newEntity = std::make_unique<Entity>(*this, id);
    Entity *entityPtr = newEntity.get();
    entities.push_back(std::move(newEntity));
    return *entityPtr;
}

Entity *EntityManager::getEntityByID(EntityID id)
{
    for (auto &entity : entities) {
        if (entity && entity->getID() == id)
            return entity.get();
    }
    return nullptr;
}

void EntityManager::refresh()
{
    // Supprimer les entités inactives
    entities.erase(std::remove_if(entities.begin(), entities.end(),
                                  [](const std::unique_ptr<Entity> &entity) {
                                      return !entity || !entity->isActive();
                                  }),
                   entities.end());

    // Mettre à jour les listes d'entités par composant
    for (auto &componentEntities : entitiesByComponent)
        componentEntities.clear();

    for (auto &entity : entities) {
        if (entity && entity->isActive()) {
            for (ComponentID i = 0; i < MAX_COMPONENTS; i++) {
                if (entity->getComponentMask().test(i))
                    entitiesByComponent[i].push_back(entity.get());
            }
        }
    }
}



// === CLIENT - Désérialisation complète d'une entité (TCP) ===
void EntityManager::deserializeEntityFull(const std::vector<uint8_t> &data)
{
    if (data.empty())
        return;

    auto &newEntity = createEntity();
    newEntity.deserialize(data.data(), data.size());
}






// === CLIENT - Désérialisation des mouvements (UDP) ===
void EntityManager::deserializeAllMovements(const std::vector<uint8_t> &data)
{
    if (data.size() < sizeof(uint32_t))
        return;

    size_t offset = 0;

    // Lire le nombre d'entités
    uint32_t entityCount;
    std::memcpy(&entityCount, data.data() + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    // Pour chaque entité
    for (uint32_t i = 0; i < entityCount && offset < data.size(); ++i)
    {
        // Lire EntityID
        EntityID id;
        std::memcpy(&id, data.data() + offset, sizeof(EntityID));
        offset += sizeof(EntityID);

        // Trouver l'entité correspondante
        Entity *entity = getEntityByID(id);
        if (!entity)
        {
            // Ignorer les données de cette entité
            offset += 2 * sizeof(Vector2D); // position + velocity
            continue;
        }

        // Lire position
        Vector2D position = Vector2D::deserialize(data.data() + offset, sizeof(Vector2D));
        offset += sizeof(Vector2D);

        // Lire velocity
        Vector2D velocity = Vector2D::deserialize(data.data() + offset, sizeof(Vector2D));
        offset += sizeof(Vector2D);

        // Mettre à jour les composants
        if (entity->hasComponent<TransformComponent>())
        {
            auto &transform = entity->getComponent<TransformComponent>();
            transform.position = position;
        }

        if (entity->hasComponent<VelocityComponent>())
        {
            auto &vel = entity->getComponent<VelocityComponent>();
            vel.velocity = velocity;
        }
    }
}

// === CLIENT - Détruire une entité par ID ===
void EntityManager::destroyEntityByID(EntityID id)
{
    for (auto &entity : entities)
    {
        if (entity && entity->getID() == id)
        {
            entity->destroy();
            return;
        }
    }
}

// === SERVEUR - Marquer pour destruction ===
void EntityManager::markEntityForDestruction(EntityID id)
{
    entitiesToDestroy.push_back(id);
}

// === SERVEUR - Queue création ===
Entity& EntityManager::queueEntityCreation()
{
    EntityID id = static_cast<EntityID>(entities.size() + entitiesToCreate.size());
    auto newEntity = std::make_unique<Entity>(*this, id);
    Entity *entityPtr = newEntity.get();
    entitiesToCreate.push_back(std::move(newEntity));
    return *entityPtr;
}

// === COMMUN - Appliquer les changements ===
void EntityManager::applyPendingChanges()
{
    // Ajouter les nouvelles entités
    for (auto &entity : entitiesToCreate)
    {
        entities.push_back(std::move(entity));
    }
    entitiesToCreate.clear();

    // Détruire les entités marquées
    for (EntityID id : entitiesToDestroy)
    {
        destroyEntityByID(id);
    }
    entitiesToDestroy.clear();

    refresh();
}