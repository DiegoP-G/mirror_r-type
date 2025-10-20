#include "RTypeServer.hpp"

void RTypeServer::sendNewEntities()
{
    auto &manager = entityManager;
    std::vector<uint8_t> data;

    uint32_t tick = _tick;
    // Ajouter le tick au début
    data.insert(data.end(), reinterpret_cast<uint8_t *>(&tick), reinterpret_cast<uint8_t *>(&tick) + sizeof(tick));

    // Nombre d'entités à envoyer
    uint32_t entityCount = static_cast<uint32_t>(manager.getEntitiesToCreate().size());
    if (entityCount == 0)
        return;
    data.insert(data.end(), reinterpret_cast<uint8_t *>(&entityCount),
                reinterpret_cast<uint8_t *>(&entityCount) + sizeof(entityCount));

    // Sérialiser chaque entité dans le buffer
    for (const auto &entity : manager.getEntitiesToCreate())
    {
        auto entityData = manager.serializeEntityFull(entity->getID());

        // Taille de l'entité
        uint32_t entitySize = static_cast<uint32_t>(entityData.size());
        data.insert(data.end(), reinterpret_cast<uint8_t *>(&entitySize),
                    reinterpret_cast<uint8_t *>(&entitySize) + sizeof(entitySize));

        // Données de l'entité
        data.insert(data.end(), entityData.begin(), entityData.end());
    }
    // Envoi via le mediator (ou directement via ton réseau)
    std::string serializedData(data.begin(), data.end());
    mediator.notify(GameMediatorEvent::EntitiesCreated, serializedData);
}

void RTypeServer::sendDestroyedEntities()
{
    auto &manager = entityManager;

    // Pour chaque entité détruite ce tick
    for (EntityID id : manager.getEntitiesToDestroy())
    {
        std::cout << "--DESTROY id " << id << std::endl;
        auto data = serializeInt(id);
        mediator.notify(GameMediatorEvent::EntityDestroyed, data);
    }
}

void RTypeServer::sendMovementUpdates()
{
    auto data = entityManager.serializeAllMovements();
    std::string serializedData(data.begin(), data.end());
    mediator.notify(GameMediatorEvent::MovementUpdate, serializedData);
}

void RTypeServer::sendHealthUpdates()
{
    auto data = entityManager.serializeAllHealth();
    std::string serializedData(data.begin(), data.end());
    mediator.notify(GameMediatorEvent::HealthUpdate, serializedData);
}

void RTypeServer::sendGameStateUpdates()
{
    mediator.notify(GameMediatorEvent::GameStateUpdate, serializeInt(_state));
}

void RTypeServer::sendEntitiesUpdates()
{
    std::vector<uint8_t> data;

    uint32_t tick = _tick;
    data.insert(data.end(), reinterpret_cast<uint8_t *>(&tick), reinterpret_cast<uint8_t *>(&tick) + sizeof(tick));

    auto moveData = entityManager.serializeAllMovements();
    uint16_t moveSize = static_cast<uint16_t>(moveData.size());
    data.insert(data.end(), reinterpret_cast<uint8_t *>(&moveSize),
                reinterpret_cast<uint8_t *>(&moveSize) + sizeof(moveSize));
    data.insert(data.end(), moveData.begin(), moveData.end());

    auto healthData = entityManager.serializeAllHealth();
    uint16_t healthSize = static_cast<uint16_t>(healthData.size());
    data.insert(data.end(), reinterpret_cast<uint8_t *>(&healthSize),
                reinterpret_cast<uint8_t *>(&healthSize) + sizeof(healthSize));
    data.insert(data.end(), healthData.begin(), healthData.end());

    std::string serializedData(data.begin(), data.end());
    mediator.notify(GameMediatorEvent::AllEntitiesUpdates, serializedData);
}
