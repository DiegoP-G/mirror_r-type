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
        std::cout << "caca" << std::endl;
        onCodeUdp(payload);
    };
    _handlers[OPCODE_CLOSE_CONNECTION] = [this](const std::string &payload, int opcode) { onCloseConnection(payload); };
    _handlers[OPCODE_CHAT_BROADCAST] = [this](const std::string &payload, int opcode) { onChatBroadcast(payload); };
    _handlers[OPCODE_WORLD_UPDATE] = [this](const std::string &payload, int opcode) {
        _med.notify(UPDATE_DATA, payload, opcode);
    };
    _handlers[OPCODE_ENEMIES_UPDATE] = [this](const std::string &payload, int opcode) {
        _med.notify(UPDATE_DATA, payload, opcode);
    };
    _handlers[OPCODE_PROJECTILES_UPDATE] = [this](const std::string &payload, int opcode) {
        _med.notify(UPDATE_DATA, payload, opcode);
    };
    _handlers[OPCODE_PLAYER_UPDATE] = [this](const std::string &payload, int opcode) {
        _med.notify(UPDATE_DATA, payload, opcode);
    };
    _handlers[OPCODE_PLAYER_ID] = [this](const std::string &payload, int opcode) {
        _med.notify(PLAYER_ID, payload, opcode);
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
    if (_tcpSocket == -1)
        return;

    std::string tmp;
    auto [opcode, payload] = receiveFrameTCP(_tcpSocket, tmp);

    // std::cout << payload.length() << std::endl;
    auto it = _handlers.find(opcode);
    // std::cout << "PAYLOAD TCP: " << payload << " OPCODE: " << static_cast<int>(opcode) << std::endl;
    if (it != _handlers.end())
        it->second(payload, opcode);
    else
        std::cerr << "[RECEIVER] No handler for opcode " << (int)opcode << std::endl;
}

void Receiver::receiveUDPMessage()
{
    if (_udpSocket == -1)
    {
        std::cerr << "[RECEIVER] Cannot receive UDP message, UDP socket is invalid!" << std::endl;
        return;
    }

    sockaddr_in client{};
    socklen_t len = sizeof(client);
    // std::cout << "x" << std::endl;
    auto [opcode, payload] = receiveFrameUDP(_udpSocket, client, len);

    // std::cout << "[RECEIVER] UDP message received: opcode=" << std::to_string(opcode) << " | payload=" << payload
    //   << std::endl;

    auto it = _handlers.find(opcode);
    if (it != _handlers.end())
    {
        it->second(payload, opcode);
    }
    else
    {
        std::cerr << "[RECEIVER] No handler registered for opcode " << std::to_string(opcode) << std::endl;
    }
}
