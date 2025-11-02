#include "entityManager.hpp"
#include "ComponentFactory.hpp"
#include "components/AnimatedSpriteComponent.hpp"
#include "components/BackgroundScrollComponent.hpp"
#include "components/BonusComponent.hpp"
#include "components/CenteredComponent.hpp"
#include "components/CircularMotionComponent.hpp"
#include "components/ColliderComponent.hpp"
#include "components/EnemyComponent.hpp"
#include "components/GameStateComponent.hpp"
#include "components/GravityComponent.hpp"
#include "components/HealthBarComponent.hpp"
#include "components/HealthComponent.hpp"
#include "components/JumpComponent.hpp"
#include "components/LaserWarningComponent.hpp"
#include "components/PipeComponent.hpp"
#include "components/PlayerComponent.hpp"
#include "components/ProjectileComponent.hpp"
#include "components/ShieldComponent.hpp"
#include "components/SpriteComponent.hpp"
#include "components/TextComponent.hpp"
#include "components/TransformComponent.hpp"
#include "components/VelocityComponent.hpp"

#include <algorithm>

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
    ComponentFactory::registerComponent<HealthBarComponent>();
    ComponentFactory::registerComponent<BonusComponent>();
    ComponentFactory::registerComponent<TextComponent>();
    ComponentFactory::registerComponent<ShieldComponent>();
    ComponentFactory::registerComponent<TextComponent>();
    ComponentFactory::registerComponent<CircularMotionComponent>();
}

std::vector<uint8_t> EntityManager::serializeEntityFull(EntityID id) const
{
    for (const auto &entity : entities)
    {
        if (entity && entity->isActive() && entity->getID() == id)
        {
            return entity->serialize();
        }
    }
    for (const auto &entity : entitiesToCreate)
    {
        if (entity && entity->getID() == id)
        {
            return entity->serialize();
        }
    }
    return {};
}

std::vector<uint8_t> EntityManager::serializeAllMovements() const
{
    std::vector<uint8_t> data;
    uint32_t entityCount = 0;
    for (const auto &entity : entities)
    {
        if (entity && entity->isActive())
            entityCount++;
    }

    data.insert(data.end(), reinterpret_cast<const uint8_t *>(&entityCount),
                reinterpret_cast<const uint8_t *>(&entityCount) + sizeof(uint32_t));

    for (const auto &entity : entities)
    {
        if (!entity || !entity->isActive() || entity->hasComponent<BackgroundScrollComponent>())
            continue;

        EntityID id = entity->getID();
        data.insert(data.end(), reinterpret_cast<const uint8_t *>(&id),
                    reinterpret_cast<const uint8_t *>(&id) + sizeof(EntityID));

        if (entity->hasComponent<TransformComponent>())
        {
            auto &transform = entity->getComponent<TransformComponent>();
            auto posData = transform.position.serialize();
            data.insert(data.end(), posData.begin(), posData.end());
        }
        else
        {
            Vector2D zero(0, 0);
            auto posData = zero.serialize();
            data.insert(data.end(), posData.begin(), posData.end());
        }

        if (entity->hasComponent<VelocityComponent>())
        {
            auto &velocity = entity->getComponent<VelocityComponent>();
            auto velData = velocity.velocity.serialize();
            data.insert(data.end(), velData.begin(), velData.end());
        }
        else
        {
            Vector2D zero(0, 0);
            auto velData = zero.serialize();
            data.insert(data.end(), velData.begin(), velData.end());
        }
    }

    return data;
}

Entity &EntityManager::createEntity(int newID)
{
    static int nextAutoID = 1000;
    EntityID id;

    if (newID == -1)
    {
        id = nextAutoID++;
    }
    else
    {
        id = newID;
        if (id >= nextAutoID)
        {
            nextAutoID = id + 1;
        }
    }

    auto newEntity = std::make_unique<Entity>(*this, id);
    Entity *entityPtr = newEntity.get();
    entitiesToCreate.push_back(std::move(newEntity));

    return *entityPtr;
}

Entity *EntityManager::getEntityByID(EntityID id)
{
    for (auto &entity : entities)
    {
        if (entity && entity->getID() == id)
            return entity.get();
    }
    return nullptr;
}

std::vector<Entity *> EntityManager::getPlayersDead(int &winnerID, bool &game_over)
{
    std::vector<Entity *> deadPlayers;
    auto players = getAllEntitiesWithComponents<PlayerComponent>();

    for (auto *entity : players)
    {
        if (entity->hasComponent<HealthComponent>())
        {
            auto &healthComp = entity->getComponent<HealthComponent>();
            if (healthComp.health <= 0)
            {
                deadPlayers.push_back(entity);
            }
        }
    }
    if (deadPlayers.size() == players.size() - 1 && players.size() > 1)
    {
        game_over = true;
        for (auto *entity : players)
        {
            if (std::find(deadPlayers.begin(), deadPlayers.end(), entity) == deadPlayers.end())
            {
                auto &playerComp = entity->getComponent<PlayerComponent>();
                winnerID = playerComp.playerID;
                break;
            }
        }
    }
    else if (deadPlayers.size() == players.size() && players.size() > 0)
    {
        game_over = true;
        winnerID = -1;
    }
    return deadPlayers;
}

void EntityManager::refresh()
{
    //     entities.erase(std::remove_if(entities.begin(), entities.end(),
    //                                   [](const std::unique_ptr<Entity> &entity) { return !entity ||
    //                                   !entity->isActive(); }),
    //                    entities.end());

    for (auto &componentEntities : entitiesByComponent)
        componentEntities.clear();

    for (auto &entity : entities)
    {
        if (entity && entity->isActive())
        {
            for (ComponentID i = 0; i < MAX_COMPONENTS; i++)
            {
                if (entity->getComponentMask().test(i))
                    entitiesByComponent[i].push_back(entity.get());
            }
        }
    }
}

void EntityManager::deserializeEntityFull(const std::vector<uint8_t> &data)
{
    if (data.empty())
        return;

    auto &newEntity = createEntity();
    size_t bytesRead = newEntity.deserialize(data.data(), data.size());

    std::cout << "\n========== ENTITY DESERIALIZED ==========" << std::endl;
    std::cout << "Entity ID: " << newEntity.getID() << std::endl;
    std::cout << "Bytes read: " << bytesRead << " / " << data.size() << std::endl;

    std::string entityType = "UNKNOWN";

    if (newEntity.hasComponent<PlayerComponent>())
    {
        auto &playerComp = newEntity.getComponent<PlayerComponent>();
        entityType = "PLAYER";
        std::cout << "✅ Type: PLAYER" << std::endl;
        std::cout << "   - PlayerID: " << playerComp.playerID << std::endl;
        std::cout << "   - IsLocal: " << playerComp.isLocal << std::endl;
        std::cout << "   - Score: " << playerComp.score << std::endl;
        std::cout << "   - Lives: " << playerComp.lives << std::endl;
    }

    if (newEntity.hasComponent<BackgroundScrollComponent>())
    {
        auto &bgComp = newEntity.getComponent<BackgroundScrollComponent>();
        entityType = "BACKGROUND";
        std::cout << "✅ Type: BACKGROUND" << std::endl;
        std::cout << "   - Scroll Speed: " << bgComp.scrollSpeed << std::endl;
        std::cout << "   - Texture: " << bgComp.texture << std::endl;
    }

    if (newEntity.hasComponent<EnemyComponent>())
    {
        auto &enemyComp = newEntity.getComponent<EnemyComponent>();
        entityType = "ENEMY";
        std::cout << "✅ Type: ENEMY" << std::endl;
        std::cout << "   - Enemy Type: " << enemyComp.type << std::endl;
        std::cout << "   - Shooting Type: " << enemyComp.shootingType << std::endl;
        std::cout << "   - Attack Cooldown: " << enemyComp.attackCooldown << std::endl;
    }

    if (newEntity.hasComponent<ProjectileComponent>())
    {
        auto &projComp = newEntity.getComponent<ProjectileComponent>();
        entityType = "PROJECTILE";
        std::cout << "✅ Type: PROJECTILE" << std::endl;
        std::cout << "   - Damage: " << projComp.damage << std::endl;
        std::cout << "   - Owner ID: " << projComp.owner_id << std::endl;
        std::cout << "   - Life Time: " << projComp.lifeTime << std::endl;
        std::cout << "   - Remaining Life: " << projComp.remainingLife << std::endl;
    }

    // Composants communs
    if (newEntity.hasComponent<TransformComponent>())
    {
        auto &transform = newEntity.getComponent<TransformComponent>();
        std::cout << "📍 Transform: (" << transform.position.x << ", " << transform.position.y << ")" << std::endl;
    }

    if (newEntity.hasComponent<VelocityComponent>())
    {
        auto &velocity = newEntity.getComponent<VelocityComponent>();
        std::cout << "🚀 Velocity: (" << velocity.velocity.x << ", " << velocity.velocity.y << ")" << std::endl;
    }

    if (newEntity.hasComponent<SpriteComponent>())
    {
        auto &sprite = newEntity.getComponent<SpriteComponent>();
        std::cout << "🎨 Sprite: " << sprite.width << "x" << sprite.height << " RGB(" << (int)sprite.r << ","
                  << (int)sprite.g << "," << (int)sprite.b << ")" << std::endl;
        std::cout << "   - Texture ID: " << sprite.spriteTexture << std::endl;
    }

    if (newEntity.hasComponent<ColliderComponent>())
    {
        auto &collider = newEntity.getComponent<ColliderComponent>();
        std::cout << "💥 Collider: " << collider.hitbox.w << "x" << collider.hitbox.h
                  << " (active: " << collider.isActive << ")" << std::endl;
    }

    if (newEntity.hasComponent<AnimatedSpriteComponent>())
    {
        auto &animSprite = newEntity.getComponent<AnimatedSpriteComponent>();
        std::cout << "🎬 Animated Sprite: frame " << animSprite.currentFrame
                  << ", interval: " << animSprite.animationInterval << std::endl;
    }

    if (newEntity.hasComponent<InputComponent>())
    {
        std::cout << "🎮 Has InputComponent" << std::endl;
    }

    if (newEntity.hasComponent<HealthComponent>())
    {
        auto &health = newEntity.getComponent<HealthComponent>();
        std::cout << "❤️  Health: " << health.health << " / " << health.maxHealth << std::endl;
    }

    // Résumé du masque de composants
    std::cout << "Component Mask: " << newEntity.getComponentMask() << std::endl;

    // Compter les composants
    int componentCount = 0;
    for (ComponentID i = 0; i < MAX_COMPONENTS; ++i)
    {
        if (newEntity.hasComponent<TransformComponent>() && i == getComponentTypeID<TransformComponent>())
            componentCount++;
        if (newEntity.hasComponent<PlayerComponent>() && i == getComponentTypeID<PlayerComponent>())
            componentCount++;
        if (newEntity.hasComponent<EnemyComponent>() && i == getComponentTypeID<EnemyComponent>())
            componentCount++;
        if (newEntity.hasComponent<ProjectileComponent>() && i == getComponentTypeID<ProjectileComponent>())
            componentCount++;
        if (newEntity.hasComponent<VelocityComponent>() && i == getComponentTypeID<VelocityComponent>())
            componentCount++;
        if (newEntity.hasComponent<SpriteComponent>() && i == getComponentTypeID<SpriteComponent>())
            componentCount++;
        if (newEntity.hasComponent<ColliderComponent>() && i == getComponentTypeID<ColliderComponent>())
            componentCount++;
        if (newEntity.hasComponent<InputComponent>() && i == getComponentTypeID<InputComponent>())
            componentCount++;
        if (newEntity.hasComponent<HealthComponent>() && i == getComponentTypeID<HealthComponent>())
            componentCount++;
        if (newEntity.hasComponent<AnimatedSpriteComponent>() && i == getComponentTypeID<AnimatedSpriteComponent>())
            componentCount++;
    }

    std::cout << "Total components: " << componentCount << std::endl;
    std::cout << "========================================\n" << std::endl;
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

std::vector<uint8_t> EntityManager::serializeAllHealth() const
{
    std::vector<uint8_t> data;
    uint32_t entityCount = 0;

    // Compter les entités avec HealthComponent
    for (const auto &entity : entities)
    {
        if (entity && entity->isActive() && entity->hasComponent<HealthComponent>())
            entityCount++;
    }

    // Écrire le nombre d'entités
    data.insert(data.end(), reinterpret_cast<const uint8_t *>(&entityCount),
                reinterpret_cast<const uint8_t *>(&entityCount) + sizeof(entityCount));

    // Écrire chaque entité
    for (const auto &entity : entities)
    {
        if (!entity || !entity->isActive() || !entity->hasComponent<HealthComponent>())
            continue;

        EntityID id = entity->getID();
        data.insert(data.end(), reinterpret_cast<const uint8_t *>(&id),
                    reinterpret_cast<const uint8_t *>(&id) + sizeof(EntityID));

        auto &health = entity->getComponent<HealthComponent>();
        auto serialized = health.serialize();

        data.insert(data.end(), serialized.begin(), serialized.end());
    }

    return data;
}
void EntityManager::deserializeAllHealth(const std::vector<uint8_t> &data)
{
    if (data.size() < sizeof(uint32_t))
        return;

    size_t offset = 0;
    uint32_t entityCount = 0;

    std::memcpy(&entityCount, data.data() + offset, sizeof(entityCount));
    offset += sizeof(entityCount);

    for (uint32_t i = 0; i < entityCount && offset < data.size(); ++i)
    {
        if (offset + sizeof(EntityID) > data.size())
            break;

        EntityID id;
        std::memcpy(&id, data.data() + offset, sizeof(EntityID));
        offset += sizeof(EntityID);

        Entity *entity = getEntityByID(id);
        if (!entity || !entity->hasComponent<HealthComponent>())
        {
            std::cout << "NO ENTITITY ID" << id << std::endl;
            continue;
        }

        // Vérifier qu’il reste assez d’octets pour un HealthComponent
        size_t remaining = data.size() - offset;
        if (remaining < sizeof(int) * 2)
        {
            break;
        }

        auto comp = HealthComponent::deserialize(data.data() + offset, sizeof(int) * 2);
        offset += sizeof(int) * 2;

        auto &health = entity->getComponent<HealthComponent>();
        health.health = comp.health;
        health.maxHealth = comp.maxHealth;
    }
}

// === CLIENT - Détruire une entité par ID ===
void EntityManager::destroyEntityByID(EntityID id)
{
    for (auto it = entities.begin(); it != entities.end(); ++it)
    {
        if (*it && (*it)->getID() == id)
        {
            entities.erase(it); // unique_ptr automatically deletes the Entity
            return;
        }
    }
}

// === SERVEUR - Marquer pour destruction ===
void EntityManager::markEntityForDestruction(EntityID id)
{
    entitiesToDestroy.push_back(id);
}

// === COMMUN - Appliquer les changements ===
void EntityManager::applyPendingChanges()
{
    // Move new entities into main container
    for (auto &entity : entitiesToCreate)
    {
        entities.push_back(std::move(entity));
    }
    entitiesToCreate.clear();

    // Destroy entities marked for deletion
    for (EntityID id : entitiesToDestroy)
    {
        destroyEntityByID(id);
    }
    entitiesToDestroy.clear();

    refresh();
}

std::vector<uint8_t> EntityManager::serializePlayersScores(const std::vector<std::pair<int, int>> &playersScores)
{
    std::vector<uint8_t> data;
    size_t totalSize = playersScores.size() * (sizeof(int) * 2);
    data.resize(totalSize);

    size_t offset = 0;
    for (const auto &pair : playersScores)
    {
        std::memcpy(data.data() + offset, &pair.first, sizeof(int));
        offset += sizeof(int);
        std::memcpy(data.data() + offset, &pair.second, sizeof(int));
        offset += sizeof(int);
    }

    return data;
}

std::vector<std::pair<int, int>> EntityManager::deserializePlayersScores(const std::vector<uint8_t> &data)
{
    std::vector<std::pair<int, int>> playersScores;

    if (data.size() % (sizeof(int) * 2) != 0)
    {
        throw std::runtime_error("Invalid data size for deserializing players' scores");
    }

    size_t offset = 0;
    while (offset < data.size())
    {
        int playerID;
        int score;

        std::memcpy(&playerID, data.data() + offset, sizeof(int));
        offset += sizeof(int);

        std::memcpy(&score, data.data() + offset, sizeof(int));
        offset += sizeof(int);

        playersScores.emplace_back(playerID, score);
    }

    return playersScores;
}

std::vector<uint8_t> EntityManager::serializeAllShields() const
{
    std::vector<uint8_t> data;
    uint32_t entityCount = 0;

    for (const auto &entity : entities)
    {
        if (entity && entity->isActive() && entity->hasComponent<ShieldComponent>())
            entityCount++;
    }

    data.insert(data.end(), (const uint8_t *)&entityCount, (const uint8_t *)&entityCount + sizeof(entityCount));

    for (const auto &entity : entities)
    {
        if (!entity || !entity->isActive() || !entity->hasComponent<ShieldComponent>())
            continue;

        EntityID id = entity->getID();
        data.insert(data.end(), (const uint8_t *)&id, (const uint8_t *)&id + sizeof(EntityID));

        auto &shield = entity->getComponent<ShieldComponent>();
        auto serialized = shield.serialize();
        data.insert(data.end(), serialized.begin(), serialized.end());
    }

    return data;
}

void EntityManager::deserializeAllShields(const std::vector<uint8_t> &data)
{
    if (data.size() < sizeof(uint32_t))
        return;

    size_t offset = 0;
    uint32_t entityCount = 0;

    std::memcpy(&entityCount, data.data() + offset, sizeof(entityCount));
    offset += sizeof(entityCount);

    for (uint32_t i = 0; i < entityCount && offset < data.size(); i++)
    {
        if (offset + sizeof(EntityID) > data.size())
            break;

        EntityID id;
        std::memcpy(&id, data.data() + offset, sizeof(EntityID));
        offset += sizeof(EntityID);

        Entity *entity = getEntityByID(id);
        if (!entity || !entity->hasComponent<ShieldComponent>())
            continue;

        size_t remaining = data.size() - offset;
        size_t expectedSize = sizeof(int) + sizeof(float);
        if (remaining < expectedSize)
            break;

        auto comp = ShieldComponent::deserialize(data.data() + offset, expectedSize);
        offset += expectedSize;

        auto &shield = entity->getComponent<ShieldComponent>();
        shield.ownerID = comp.ownerID;
        shield.shieldLeft = comp.shieldLeft;
    }
}
