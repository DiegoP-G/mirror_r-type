#include "Receiver.hpp"
#include "../../transferData/opcode.hpp"
#include "../../transferData/transferData.hpp"
#include "../NetworkECSMediator.hpp"
#include <arpa/inet.h>
#include <iostream>
#include <unistd.h>

Receiver::Receiver(NetworkECSMediator &med) : _med(med)
{
    _handlers[OPCODE_CODE_UDP] = [this](const std::string &payload, int opcode) { onCodeUdp(payload); };

    _handlers[OPCODE_CLOSE_CONNECTION] = [this](const std::string &payload, int opcode) { onCloseConnection(payload); };

    _handlers[OPCODE_CHAT_BROADCAST] = [this](const std::string &payload, int opcode) { onChatBroadcast(payload); };

    _handlers[OPCODE_ENTITY_CREATE] = [this](const std::string &payload, int opcode) {
        _med.notify(NetworkECSMediatorEvent::UPDATE_DATA, payload, opcode);
    };

    _handlers[OPCODE_ENTITY_DESTROY] = [this](const std::string &payload, int opcode) {
        _med.notify(NetworkECSMediatorEvent::UPDATE_DATA, payload, opcode);
    };

    _handlers[OPCODE_MOVEMENT_UPDATE] = [this](const std::string &payload, int opcode) {
        _med.notify(NetworkECSMediatorEvent::UPDATE_DATA, payload, opcode);
    };
    _handlers[OPCODE_HEALTH_UPDATE] = [this](const std::string &payload, int opcode) {
        _med.notify(NetworkECSMediatorEvent::UPDATE_DATA, payload, opcode);
    };

    _handlers[OPCODE_UPDATE_WAVE] = [this](const std::string &payload, int opcode) {
        _med.notify(NetworkECSMediatorEvent::UPDATE_DATA, payload, opcode);
    };

    _handlers[OPCODE_UPDATE_SCORE] = [this](const std::string &payload, int opcode) {
        _med.notify(NetworkECSMediatorEvent::UPDATE_DATA, payload, opcode);
    };

    _handlers[OPCODE_PLAYER_ID] = [this](const std::string &payload, int opcode) {
        _med.notify(PLAYER_ID, payload, opcode);
    };

    _handlers[OPCODE_LOBBY_INFO] = [this](const std::string &payload, int opcode) {
        _med.notify(UPDATE_DATA, payload, opcode);
    };

    _handlers[OPCODE_GAME_OVER] = [this](const std::string &payload, int opcode) {
        _med.notify(UPDATE_DATA, payload, opcode);
    };
    _handlers[OPCODE_PLAYER_DEAD] = [this](const std::string &payload, int opcode) {
        _med.notify(UPDATE_DATA, payload, opcode);
    };
    _handlers[OPCODE_GAME_STATE_UPDATE] = [this](const std::string &payload, int opcode) {
        _med.notify(UPDATE_DATA, payload, opcode);
    };
    _handlers[OPCODE_KICK_NOTIFICATION] = [this](const std::string &payload, int opcode) {
        _med.notify(UPDATE_DATA, payload, opcode);
    };
    _handlers[OPCODE_BAN_NOTIFICATION] = [this](const std::string &payload, int opcode) {
        _med.notify(UPDATE_DATA, payload, opcode);
    };
    _handlers[OPCODE_LOGIN_REQUEST] = [this](const std::string &payload, int opcode) {
        _med.notify(SEND_DATA_TCP, payload, opcode);
    };
    _handlers[OPCODE_LOGIN_RESPONSE] = [this](const std::string &payload, int opcode) {
        _med.notify(SEND_DATA_TCP, payload, opcode);
    };
    _handlers[OPCODE_SIGNIN_REQUEST] = [this](const std::string &payload, int opcode) {
        _med.notify(SEND_DATA_TCP, payload, opcode);
    };
    _handlers[OPCODE_SIGNIN_RESPONSE] = [this](const std::string &payload, int opcode) {
        _med.notify(SEND_DATA_TCP, payload, opcode);
    };
    _handlers[OPCODE_SERVER_PUB_KEY] = [this](const std::string &payload, int opcode) {
        _med.notify(SEND_DATA_TCP, payload, opcode);
    };
}

void Receiver::onCodeUdp(const std::string &payload)
{
    if (_udpSocket == -1)
    {
        std::cerr << "[RECEIVER] UDP socket not initialized" << std::endl;
        return;
    }

    int udpCode = deserializeInt(payload);
    std::cout << "[RECEIVER] UDP code received: " << udpCode << std::endl;

    sendFrameUDP(_udpSocket, OPCODE_UDP_AUTH, payload, _serverAddr, sizeof(_serverAddr));
}

void Receiver::onCloseConnection(const std::string &)
{
    std::cout << "[RECEIVER] Server closed connection" << std::endl;
    if (_tcpSocket != -1)
    {
        close(_tcpSocket);
        _tcpSocket = -1;
    }
}

void Receiver::onChatBroadcast(const std::string &payload)
{
    std::cout << "[RECEIVER] Chat: " << payload << std::endl;
}

void Receiver::receiveTCPMessage()
{
    if (_tcpSocket == -1)
        return;

    while (true)
    {
        auto [opcode, payload] = receiveFrameTCP(_tcpSocket, _tcpBuffer);

        if (opcode == OPCODE_INCOMPLETE_DATA)
        {
            break;
        }

        if (opcode == OPCODE_CLOSE_CONNECTION)
        {
            onCloseConnection("");
            break;
        }

        std::cout << "[RECEIVER] TCP message: opcode=0x" << std::hex << (int)opcode << std::dec << ", "
                  << payload.size() << " bytes" << std::endl;

        auto it = _handlers.find(opcode);
        if (it != _handlers.end())
        {
            it->second(payload, opcode);
        }
        else
        {
            std::cerr << "[RECEIVER] Unknown opcode: 0x" << std::hex << (int)opcode << std::dec << std::endl;
        }
    }
}

void Receiver::receiveUDPMessage()
{
    if (_udpSocket == -1)
        return;

    sockaddr_in client{};
    socklen_t len = sizeof(client);

    auto [opcode, payload] = receiveFrameUDP(_udpSocket, client, len);

    auto it = _handlers.find(opcode);
    if (it != _handlers.end())
    {
        it->second(payload, opcode);
    }
}