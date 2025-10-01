#include "Receiver.hpp"
#include "../../transferData/opcode.hpp"
#include "../../transferData/transferData.hpp"
#include "../NetworkECSMediator.hpp"
#include <arpa/inet.h>
#include <cstdio>
#include <iostream>
#include <string>
#include <unistd.h>

Receiver::Receiver(NetworkECSMediator &med) : _med(med)
{
    _handlers[OPCODE_CODE_UDP] = [this](const std::string &payload, int opcode) {
        std::cout << "[RECEIVER] Received OPCODE_CODE_UDP" << std::endl;
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
        std::cout << "[RECEIVER] Received OPCODE_PLAYER_ID" << std::endl;
        _med.notify(PLAYER_ID, payload, opcode);
    };
}

void Receiver::onCodeUdp(const std::string &payload)
{
    std::cout << "[RECEIVER] Processing UDP code, payload length: " << payload.length() << std::endl;

    if (_udpSocket == -1)
    {
        std::cerr << "[RECEIVER] Cannot send UDP auth, UDP socket is invalid!" << std::endl;
        return;
    }

    int udpCode = deserializeInt(payload);
    std::cout << "[RECEIVER] Sending UDP auth with code: " << udpCode << std::endl;

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

    while (true)
    {
        std::cout << "befroe receive frame" << std::endl;
        auto [opcode, payload] = receiveFrameTCP(_tcpSocket, _tcpBuffer);
        std::cout << "opcode is :" << opcode << std::endl;
        std::cout << _tcpBuffer << std::endl;

        // Si données incomplètes, on attend le prochain appel
        if (opcode == OPCODE_INCOMPLETE_DATA)
        {
            std::cout << "[RECEIVER] Incomplete data, waiting for more..." << std::endl;
            break;
        }

        // Si connexion fermée
        if (opcode == OPCODE_CLOSE_CONNECTION)
        {
            std::cout << "[RECEIVER] Connection closed by server" << std::endl;
            onCloseConnection("");
            break;
        }

        // Traiter le message
        std::cout << "[RECEIVER] TCP message: opcode=" << static_cast<int>(opcode)
                  << ", payload_length=" << payload.length() << std::endl;

        auto it = _handlers.find(opcode);
        if (it != _handlers.end())
        {
            std::cout << "here 0" << std::endl;
            it->second(payload, opcode);
        }
        else
        {

            std::cerr << "[RECEIVER] No handler for opcode " << static_cast<int>(opcode) << std::endl;
        }
        std::cout << "here 1" << std::endl;
        // Si le buffer est vide, on sort de la boucle
        if (_tcpBuffer.empty())
        {
            std::cout << "here" << std::endl;
            break;
        }
    }
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

    auto [opcode, payload] = receiveFrameUDP(_udpSocket, client, len);

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