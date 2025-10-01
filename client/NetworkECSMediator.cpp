#include "NetworkECSMediator.hpp"
#include "../transferData/opcode.hpp"
#include "Network/Receiver.hpp"
#include "RType.hpp"
#include <cstdint>
#include <exception>
#include <iostream>
#include <stdexcept>

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
         [this](const std::string &data, uint8_t opcode) {
             _sender->sendUdp(opcode, data);
         }},
        
        // === RÉCEPTION DEPUIS LE SERVEUR ===
        {static_cast<int>(NetworkECSMediatorEvent::UPDATE_DATA), 
         [this](const std::string &data, uint8_t opcode) {
             std::cout << "[Client] Received opcode: 0x" << std::hex << (int)opcode << std::dec << std::endl;
             
             switch (opcode)
             {
                 // Création complète d'une entité (TCP)
                 case OPCODE_ENTITY_CREATE:
                 {
                     std::cout << "[Client] Entity created" << std::endl;
                     _game->getMutex().lock();
                     std::vector<uint8_t> bytes(data.begin(), data.end());
                     _game->getEntityManager().deserializeEntityFull(bytes);
                     _game->getMutex().unlock();
                     break;
                 }
                 
                 // Destruction d'une entité (TCP)
                 case OPCODE_ENTITY_DESTROY:
                 {
                     std::cout << "[Client] Entity destroyed" << std::endl;
                     _game->getMutex().lock();
                     EntityID id = deserializeInt(data);
                     _game->getEntityManager().destroyEntityByID(id);
                     _game->getMutex().unlock();
                     break;
                 }
                 
                 // Updates de mouvement (UDP)
                 case OPCODE_MOVEMENT_UPDATE:
                 {
                     _game->getMutex().lock();
                     std::vector<uint8_t> bytes(data.begin(), data.end());
                     _game->getEntityManager().deserializeAllMovements(bytes);
                     _game->getMutex().unlock();
                     break;
                 }
                 
                 default:
                     std::cerr << "[Client] Unhandled opcode: 0x" << std::hex << (int)opcode << std::dec << std::endl;
                     break;
             }
         }}
    };
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
