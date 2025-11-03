#include "Receiver.hpp"
#include "../../transferData/opcode.hpp"
#include "../../transferData/transferData.hpp"
#include "../NetworkECSMediator.hpp"
#include "../RType.hpp"
#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

#include <BaseTsd.h>
#include <cstdint>
#include <windows.h>
typedef SSIZE_T ssize_t;
// #include <win32_port.h>

#else
#include <arpa/inet.h>
#include <netinet/in.h>
#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

uint32_t lastSeq = 0;

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

    _handlers[OPCODE_UPDATE_ENTITIES] = [this](const std::string &payload, int opcode) {
        _med.notify(NetworkECSMediatorEvent::UPDATE_DATA, payload, opcode);
    };

    _handlers[OPCODE_UPDATE_ENTITIES_ZLIB] = [this](const std::string &payload, int opcode) {
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
    _handlers[OPCODE_VOICE_DATA] = [this](const std::string &payload, int opcode) {
        _med.notify(UPDATE_DATA, payload, opcode);
    };
    _handlers[OPCODE_BONUS] = [this](const std::string &payload, int opcode) {
        _med.notify(UPDATE_DATA, payload, opcode);
    };
    _handlers[OPCODE_NEW_WAVE] = [this](const std::string &payload, int opcode) {
        _med.notify(UPDATE_DATA, payload, opcode);
    };
    _handlers[OPCODE_EXPLOSION] = [this](const std::string &payload, int opcode) {
        _med.notify(UPDATE_DATA, payload, opcode);
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
#ifdef _WIN32
        closesocket(_tcpSocket);
#else
        close(_tcpSocket);
#endif
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

    auto [opcode, payload] = receiveFrameTCP(_tcpSocket, _tcpBuffer);

    if (opcode == OPCODE_INCOMPLETE_DATA)
        return;

    if (opcode == OPCODE_CLOSE_CONNECTION)
    {
        onCloseConnection("");
        return;
    }

    std::cout << "[RECEIVER] TCP message: opcode=0x" << std::hex << (int)opcode << std::dec << ", " << payload.size()
              << " bytes" << std::endl;

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

// std::tuple<uint8_t, std::string> Receiver::receiveFrameUDPPacketLossCount(SOCKET sockfd, struct sockaddr_in &addr,
// socklen_t &addrlen)
// {
//     char buffer[32000];
//     ssize_t n = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&addr, &addrlen);

//     if (n <= 0)
//         return {OPCODE_CLOSE_CONNECTION, ""};

//     uint8_t opcode = buffer[0];
//     std::string payload(buffer + 1, n - 1);
//     return {opcode, payload};
// }

void Receiver::receiveUDPMessage()
{
    if (_udpSocket == -1)
        return;

    sockaddr_in client{};
    socklen_t len = sizeof(client);

    auto [opcode, payload] = receiveFrameUDP(_udpSocket, client, len);

    if (payload.size() < sizeof(uint32_t))
    {
        std::cout << "returning size too small corupt" << std::endl;
        return;
    }

    uint32_t seq;
    std::memcpy(&seq, payload.data(), sizeof(uint32_t));

    std::cout << "Seq is this " << seq << std::endl;
        if (opcode != OPCODE_UPDATE_ENTITIES_ZLIB) {
            if (seq != lastSeq + 1 && lastSeq != 0)
        {
            _med.getRTypeGame()->setPacketLoss(true);
        }
        else
        {
            _med.getRTypeGame()->setPacketLoss(false);
        }
        lastSeq = seq;
    }
    
    std::cout << "Payload size:" << payload.size() << std::endl;

    auto it = _handlers.find(opcode);
    if (it != _handlers.end())
    {
        it->second(payload, opcode);
    }
}