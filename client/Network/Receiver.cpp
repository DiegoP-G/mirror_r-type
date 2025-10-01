#include "Receiver.hpp"
#include "../../transferData/opcode.hpp"
#include "../../transferData/transferData.hpp"
#include "../NetworkECSMediator.hpp"
#include <arpa/inet.h>
#include <iostream>
#include <string>
#include <unistd.h>

Receiver::Receiver(NetworkECSMediator &med) : _med(med)
{
    _handlers[OPCODE_CODE_UDP] = [this](const std::string &payload, int opcode) {
        std::cout << "[Receiver] UDP auth code received" << std::endl;
        onCodeUdp(payload);
    };
    
    _handlers[OPCODE_CLOSE_CONNECTION] = [this](const std::string &payload, int opcode) { 
        onCloseConnection(payload); 
    };
    
    _handlers[OPCODE_CHAT_BROADCAST] = [this](const std::string &payload, int opcode) { 
        onChatBroadcast(payload); 
    };
    
    // === Nouveaux handlers pour l'architecture client-serveur ===
    
    // Création d'entité (TCP)
    _handlers[OPCODE_ENTITY_CREATE] = [this](const std::string &payload, int opcode) {
        _med.notify(NetworkECSMediatorEvent::UPDATE_DATA, payload, opcode);
    };
    
    // Destruction d'entité (TCP)
    _handlers[OPCODE_ENTITY_DESTROY] = [this](const std::string &payload, int opcode) {
        _med.notify(NetworkECSMediatorEvent::UPDATE_DATA, payload, opcode);
    };
    
    // Updates de mouvement (UDP)
    _handlers[OPCODE_MOVEMENT_UPDATE] = [this](const std::string &payload, int opcode) {
        _med.notify(NetworkECSMediatorEvent::UPDATE_DATA, payload, opcode);
    };
}


void Receiver::onCodeUdp(const std::string &payload)
{
    // std::cout << "[RECEIVER] Received UDP code: " << std::to_string(deserializeInt(payload)) << std::endl;
    // std::cout << payload.length() << std::endl;
    std::cout << "penis" << std::endl;
    if (payload.length() == 0)
        return;
    if (_udpSocket == -1)
    {
        std::cerr << "[RECEIVER] Cannot send UDP auth, UDP socket is invalid!" << std::endl;
        return;
    }
    sendFrameUDP(_udpSocket, OPCODE_UDP_AUTH, payload, _serverAddr, sizeof(_serverAddr));
}

void Receiver::onCloseConnection(const std::string &)
{
    std::cerr << "[RECEIVER] Server closed TCP connection" << std::endl;
    if (_tcpSocket != -1)
    {
        close(_tcpSocket);
        _tcpSocket = -1;
    }
}

void Receiver::onChatBroadcast(const std::string &payload)
{
    std::cout << "[RECEIVER] [CHAT] " << payload << std::endl;
}

void Receiver::receiveTCPMessage()
{
    auto [opcode, payload] = receiveFrameTCP(_tcpSocket, _tcpBuffer);
    
    if (opcode == OPCODE_INCOMPLETE_DATA)
        return;
    
    if (opcode == OPCODE_CLOSE_CONNECTION)
    {
        std::cout << "[TCP] Server closed connection" << std::endl;
        return;
    }
    
    std::cout << "[TCP] Received opcode: 0x" << std::hex << (int)opcode << std::dec << std::endl;
    
    // Router vers le handler approprié
    auto it = _handlers.find(opcode);
    if (it != _handlers.end())
    {
        it->second(payload, opcode);
    }
    else
    {
        std::cout << "[TCP] Unhandled opcode: 0x" << std::hex << (int)opcode << std::dec << std::endl;
    }
}

void Receiver::receiveUDPMessage()
{
    sockaddr_in from;
    socklen_t len = sizeof(from);
    
    auto [opcode, payload] = receiveFrameUDP(_udpSocket, from, len);
    
    if (opcode == 0)
        return;
    
    // Router vers le handler approprié
    auto it = _handlers.find(opcode);
    if (it != _handlers.end())
    {
        it->second(payload, opcode);
    }
    else
    {
        std::cout << "[UDP] Unhandled opcode: 0x" << std::hex << (int)opcode << std::dec << std::endl;
    }
}