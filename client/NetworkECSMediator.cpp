#include "NetworkECSMediator.hpp"
#include "../transferData/opcode.hpp"
#include "../transferData/transferData.hpp"
#include "Network/Receiver.hpp"
#include "RType.hpp"
#include <cstdint>
#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>

NetworkECSMediator::NetworkECSMediator()
{
    _mediatorMap = {
        // === ENVOI VERS LE SERVEUR ===
        {static_cast<int>(NetworkECSMediatorEvent::SEND_DATA_TCP),
         [this](const std::string &data, uint8_t opcode) {
             std::cout << "[Client] Sending TCP opcode: 0x" << std::hex << (int)opcode << std::dec << std::endl;
             _sender->sendTcp(opcode, data);
         }},

        {static_cast<int>(NetworkECSMediatorEvent::SEND_DATA_UDP),
         [this](const std::string &data, uint8_t opcode) { _sender->sendUdp(opcode, data); }},

        {static_cast<int>(NetworkECSMediatorEvent::PLAYER_ID),
         [this](const std::string &data, uint8_t opcode) {
             std::cout << "[Client] Received PLAYER_ID" << std::endl;
             int playerId = deserializeInt(data);
             if (_game)
                 _game->setPlayerId(playerId);
         }},

        // === RÉCEPTION DEPUIS LE SERVEUR ===
        {static_cast<int>(NetworkECSMediatorEvent::UPDATE_DATA), [this](const std::string &data, uint8_t opcode) {
             //  std::cout << "[Client] Received opcode: 0x" << std::hex <<
             //  (int)opcode << std::dec << std::endl;

             switch (opcode)
             {
                 // Création complète d'une entité (TCP)
             case OPCODE_ENTITY_CREATE: {
                 std::cout << "[Client] ========== ENTITY_CREATE received ==========" << std::endl;
                 std::cout << "[Client] Data size: " << data.size() << " bytes" << std::endl;

                 _game->getMutex().lock();
                 std::vector<uint8_t> bytes(data.begin(), data.end());

                 // Log avant désérialisation
                 std::cout << "[Client] Entities before: " << _game->getEntityManager().getEntityCount() << std::endl;

                 _game->getEntityManager().deserializeEntityFull(bytes);

                 // Log après désérialisation
                 std::cout << "[Client] Entities after: " << _game->getEntityManager().getEntityCount() << std::endl;

                 _game->getMutex().unlock();
                 break;
             }

             // Destruction d'une entité (TCP)
             case OPCODE_ENTITY_DESTROY: {
                 std::cout << "[Client] Entity destroyed" << std::endl;
                 _game->getMutex().lock();
                 EntityID id = deserializeInt(data);
                 try
                 {
                     std::cout << "[Client] Marking entity " << id << " for destruction." << std::endl;
                     _game->getEntityManager().markEntityForDestruction(id);
                 }
                 catch (std::exception &e)
                 {
                     throw e.what();
                 }
                 _game->getMutex().unlock();

                 break;
             }

             case OPCODE_UPDATE_ENTITIES: {

                 _game->getMutex().lock();
                 std::vector<uint8_t> bytes(data.begin(), data.end());

                 receiveEntitiesUpdates(bytes);

                 _game->getMutex().unlock();
                 break;
             }

             case OPCODE_LOBBY_INFO: {
                 _game->getMutex().lock();
                 if (data.size() >= 2)
                 {
                     uint8_t playersReady = static_cast<uint8_t>(data[0]);
                     uint8_t totalPlayers = static_cast<uint8_t>(data[1]);

                     std::cout << "[Client] Lobby Info Received: " << static_cast<int>(playersReady) << " / "
                               << static_cast<int>(totalPlayers) << " players ready." << std::endl;

                     _game->setPlayerReady(playersReady);
                     _game->setPlayerNb(totalPlayers);
                 }
                 else
                 {
                     std::cerr << "[Client] Lobby info data too small!" << std::endl;
                 }
                 _game->getMutex().unlock();
                 //  _game->drawWaitingForPlayers((int)playersReady,
                 //  (int)totalPlayers);
                 break;
             }

             case OPCODE_UPDATE_WAVE: {
                 _game->getMutex().lock();
                 int currentWave = deserializeInt(data);
                 if (_game)
                     _game->setCurrentWave(currentWave);
                 _game->getMutex().unlock();
                 break;
             }

             case OPCODE_UPDATE_SCORE: {
                 _game->getMutex().lock();
                 std::vector<uint8_t> bytes(data.begin(), data.end());
                 std::vector<std::pair<int, int>> scoreVec = _game->getEntityManager().deserializePlayersScores(bytes);
                 if (_game)
                     _game->updateScore(scoreVec);
                 _game->getMutex().unlock();
                 break;
             }

             case OPCODE_GAME_OVER: {
                 _game->getMutex().lock();
                 int winnerId = deserializeInt(data);
                 if (_game)
                 {
                     _game->setGameOver(true);
                     _game->setWinnerId(winnerId);
                     std::cout << "[Client] Game Over! Winner ID: " << winnerId << std::endl;
                 }
                 _game->getMutex().unlock();
                 break;
             }

             case OPCODE_PLAYER_DEAD: {
                 _game->getMutex().lock();
                 int playerId = deserializeInt(data);
                 if (_game)
                 {
                     _game->markPlayerAsDead(playerId);
                     std::cout << "[Client] Player Dead! ID: " << playerId << std::endl;
                 }
                 _game->getMutex().unlock();
                 break;
             }

             case OPCODE_GAME_STATE_UPDATE: {
                 _game->getMutex().lock();
                 int state = deserializeInt(data);

                 if (_game)
                 {
                     _game->setCurrentState(static_cast<GameState>(state));
                 }
                 _game->getMutex().unlock();
                 break;
             }

             case OPCODE_KICK_NOTIFICATION: {
                 _game->getMutex().lock();
                 _game->setKickState();
                 _game->getMutex().unlock();
                 break;
             }

             case OPCODE_BAN_NOTIFICATION: {
                 _game->getMutex().lock();
                 _game->setBanState();
                 _game->getMutex().unlock();
                 break;
             }

             default:
                 std::cerr << "[Client] Unhandled opcode: 0x" << std::hex << (int)opcode << std::dec << std::endl;
                 break;
             }
         }}};
}

void NetworkECSMediator::notify(NetworkECSMediatorEvent event, const std::string &data, uint8_t opcode)
{
    auto it = _mediatorMap.find(static_cast<int>(event));

    if (it != _mediatorMap.end())
    {
        it->second(data, opcode);
    }
    else
    {
        throw std::runtime_error("notify: No handler registered for event " + std::to_string(static_cast<int>(event)));
    }
}

void NetworkECSMediator::receiveEntitiesUpdates(const std::vector<uint8_t> &data)
{
    size_t offset = 0;

    // 1. Lire le tick serveur
    uint32_t serverTick = *reinterpret_cast<const uint32_t *>(&data[offset]);
    offset += sizeof(uint32_t);

    // if (_game->getTickSystem().lastServerTick < serverTick)
    //     _game->getTickSystem().lastServerTick = serverTick;

    std::cout << " tick received: " << serverTick << std::endl;

    EntityManager serverEM; // état serveur temporaire

    uint16_t moveSize = *reinterpret_cast<const uint16_t *>(&data[offset]);
    offset += sizeof(uint16_t);

    std::vector<uint8_t> moveData(data.begin() + offset, data.begin() + offset + moveSize);
    offset += moveSize;

    deserializeMovements(moveData);

    uint16_t healthSize = *reinterpret_cast<const uint16_t *>(&data[offset]);
    offset += sizeof(uint16_t);

    std::vector<uint8_t> healthData(data.begin() + offset, data.begin() + offset + healthSize);
    deserializeHealth(healthData);

    // std::cout << "bvedoe THE IS nb " << serverEM.getEntities().size() << std::endl;
    // serverEM.applyPendingChanges();
    // std::cout << "THE IS nb " << serverEM.getEntities().size() << std::endl;
    // _game->getTickSystem().onServerUpdate(serverTick, serverEM, _game->getEntityManager());
}

void NetworkECSMediator::deserializeMovements(const std::vector<uint8_t> &data)
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
        Entity *entity = _game->getEntityManager().getEntityByID(id);
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

        // if (entity->hasComponent<PlayerComponent>() &&
        //     entity->getComponent<PlayerComponent>().playerID == _game->getPlayerId())
        // {
        //     Entity &entityServ = serverEM.createEntity(id);

        //     entityServ.addComponent<PlayerComponent>(_game->getPlayerId(), true, 0.0);
        //     entityServ.addComponent<TransformComponent>();
        //     auto &servTransform = entityServ.getComponent<TransformComponent>();
        //     auto &clientTransform = entity->getComponent<TransformComponent>();
        //     servTransform = clientTransform;
        //     servTransform.position = position;

        //     entityServ.addComponent<VelocityComponent>();

        //     auto &servVel = entityServ.getComponent<VelocityComponent>();
        //     auto &clientVel = entity->getComponent<VelocityComponent>();
        //     servVel = clientVel;
        //     servVel.velocity = velocity;

        //     std::cout << "HERE I GET MY PAYER " << std::endl;
        //     continue;
        // }

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

void NetworkECSMediator::deserializeHealth(const std::vector<uint8_t> &data)
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

        Entity *entity = _game->getEntityManager().getEntityByID(id);
        if (!entity || !entity->hasComponent<HealthComponent>())
        {
            std::cout << "NO ENTITITY ID" << id << std::endl;
            continue;
        }

        // Vérifier qu’il reste assez d’octets pour un HealthComponent
        size_t remaining = data.size() - offset;
        if (remaining < sizeof(int) * 2)
        {
            std::cout << "WTF" << std::endl;
            break;
        }

        auto comp = HealthComponent::deserialize(data.data() + offset, sizeof(int) * 2);
        offset += sizeof(int) * 2;

        auto &health = entity->getComponent<HealthComponent>();
        health.health = comp.health;
        health.maxHealth = comp.maxHealth;
    }
}
