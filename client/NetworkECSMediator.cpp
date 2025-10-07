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

NetworkECSMediator::NetworkECSMediator() {
  _mediatorMap = {
      // === ENVOI VERS LE SERVEUR ===
      {static_cast<int>(NetworkECSMediatorEvent::SEND_DATA_TCP),
       [this](const std::string &data, uint8_t opcode) {
         std::cout << "[Client] Sending TCP opcode: 0x" << std::hex
                   << (int)opcode << std::dec << std::endl;
         _sender->sendTcp(opcode, data);
       }},

      {static_cast<int>(NetworkECSMediatorEvent::SEND_DATA_UDP),
       [this](const std::string &data, uint8_t opcode) {
         _sender->sendUdp(opcode, data);
       }},

      {static_cast<int>(NetworkECSMediatorEvent::PLAYER_ID),
       [this](const std::string &data, uint8_t opcode) {
         std::cout << "[Client] Received PLAYER_ID" << std::endl;
         int playerId = deserializeInt(data);
         if (_game)
           _game->setPlayerId(playerId);
       }},

      // === RÉCEPTION DEPUIS LE SERVEUR ===
      {static_cast<int>(NetworkECSMediatorEvent::UPDATE_DATA),
       [this](const std::string &data, uint8_t opcode) {
         //  std::cout << "[Client] Received opcode: 0x" << std::hex <<
         //  (int)opcode << std::dec << std::endl;

         switch (opcode) {
           // Création complète d'une entité (TCP)
         case OPCODE_ENTITY_CREATE: {
           std::cout << "[Client] ========== ENTITY_CREATE received =========="
                     << std::endl;
           std::cout << "[Client] Data size: " << data.size() << " bytes"
                     << std::endl;

           _game->getMutex().lock();
           std::vector<uint8_t> bytes(data.begin(), data.end());

           // Log avant désérialisation
           std::cout << "[Client] Entities before: "
                     << _game->getEntityManager().getEntityCount() << std::endl;

           _game->getEntityManager().deserializeEntityFull(bytes);

           // Log après désérialisation
           std::cout << "[Client] Entities after: "
                     << _game->getEntityManager().getEntityCount() << std::endl;

           _game->getMutex().unlock();
           break;
         }

         // Destruction d'une entité (TCP)
         case OPCODE_ENTITY_DESTROY: {
           std::cout << "[Client] Entity destroyed" << std::endl;
           _game->getMutex().lock();
           EntityID id = deserializeInt(data);
           try {
             _game->getEntityManager().markEntityForDestruction(id);
           } catch (std::exception &e) {
             throw e.what();
           }
           _game->getMutex().unlock();

           break;
         }

         // Updates de mouvement (UDP)
         case OPCODE_MOVEMENT_UPDATE: {
           _game->getMutex().lock();
           std::vector<uint8_t> bytes(data.begin(), data.end());
           _game->getEntityManager().deserializeAllMovements(bytes);
           _game->getMutex().unlock();
           break;
         }

         case OPCODE_LOBBY_INFO: {
           _game->getMutex().lock();

           if (data.size() >= 2) {
             uint8_t playersReady = static_cast<uint8_t>(data[0]);
             uint8_t totalPlayers = static_cast<uint8_t>(data[1]);

             std::cout << "[Client] Lobby Info Received: "
                       << static_cast<int>(playersReady) << " / "
                       << static_cast<int>(totalPlayers) << " players ready."
                       << std::endl;

           } else {
             std::cerr << "[Client] Lobby info data too small!" << std::endl;
           }

           _game->getMutex().unlock();
           break;
         }

         default:
           std::cerr << "[Client] Unhandled opcode: 0x" << std::hex
                     << (int)opcode << std::dec << std::endl;
           break;
         }
       }}};
}

void NetworkECSMediator::notify(NetworkECSMediatorEvent event,
                                const std::string &data, uint8_t opcode) {
  auto it = _mediatorMap.find(static_cast<int>(event));

  if (it != _mediatorMap.end()) {
    it->second(data, opcode);
  } else {
    throw std::runtime_error("notify: No handler registered for event " +
                             std::to_string(static_cast<int>(event)));
  }
}
