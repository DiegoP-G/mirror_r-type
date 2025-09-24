#include "Receiver.hpp"
#include "../../transferData/opcode.hpp"
#include "../../transferData/transferData.hpp"
#include "NetworkManager.hpp"
#include <arpa/inet.h>
#include <iostream>
#include <string>
#include <unistd.h>

Receiver::Receiver(NetworkManager &ref) : _networkManagerRef(ref)
{
    _handlers[OPCODE_CODE_UDP] = [this](const std::string &payload) { onCodeUdp(payload); };
    _handlers[OPCODE_CLOSE_CONNECTION] = [this](const std::string &payload) { onCloseConnection(payload); };
    _handlers[OPCODE_CHAT_BROADCAST] = [this](const std::string &payload) { onChatBroadcast(payload); };
}

void Receiver::onCodeUdp(const std::string &payload)
{
    std::cout << "[RECEIVER] Received UDP code: " << std::to_string(deserializeInt(payload)) << std::endl;
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
    _networkManagerRef.getClientGame();
}

void Receiver::receiveTCPMessage()
{
    if (_tcpSocket == -1)
    {
        std::cerr << "[RECEIVER] Cannot receive TCP message, TCP socket is invalid!" << std::endl;
        return;
    }

    std::string buffer;
    auto [opcode, payload] = receiveFrameTCP(_tcpSocket, buffer);

    std::cout << "[RECEIVER] TCP message received: opcode=" << std::to_string(opcode) << " | payload=" << payload
              << std::endl;

    auto it = _handlers.find(opcode);
    if (it != _handlers.end())
    {
        it->second(payload);
    }
    else
    {
        std::cerr << "[RECEIVER] No handler registered for opcode " << std::to_string(opcode) << std::endl;
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

    std::cout << "[RECEIVER] UDP message received: opcode=" << std::to_string(opcode) << " | payload=" << payload
              << std::endl;

    auto it = _handlers.find(opcode);
    if (it != _handlers.end())
    {
        it->second(payload);
    }
    else
    {
        std::cerr << "[RECEIVER] No handler registered for opcode " << std::to_string(opcode) << std::endl;
    }
}
