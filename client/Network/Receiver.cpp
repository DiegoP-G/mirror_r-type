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
    _handlers[OPCODE_CODE_UDP] = [this](const std::string &payload, int opcode) { onCodeUdp(payload); };
    _handlers[OPCODE_CLOSE_CONNECTION] = [this](const std::string &payload, int opcode) { onCloseConnection(payload); };
    _handlers[OPCODE_CHAT_BROADCAST] = [this](const std::string &payload, int opcode) { onChatBroadcast(payload); };
    _handlers[OPCODE_WORLD_UPDATE] = [this](const std::string &payload, int opcode) { // THIS IS FOR TESTING
        _med.notify(UPDATE_DATA, payload, opcode);
        _med.notify(SEND_DATA_TCP, "coucouTCP", opcode);
        _med.notify(SEND_DATA_UDP, "coucouUDP", opcode);
    };
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
}

void Receiver::receiveTCPMessage()
{
    if (_tcpSocket == -1)
        return;

    char tmp[4096];
    ssize_t bytes = read(_tcpSocket, tmp, sizeof(tmp));
    if (bytes <= 0)
        return;

    _tcpBuffer.append(tmp, bytes);

    while (true)
    {
        if (_tcpBuffer.size() < 2)
            break; // need at least opcode + length
        uint8_t opcode = _tcpBuffer[0];
        size_t payloadLen = 0;

        uint8_t lenByte = _tcpBuffer[1];
        size_t headerLen = 2;

        if (lenByte <= 253)
            payloadLen = lenByte;
        else if (lenByte == 254)
        {
            if (_tcpBuffer.size() < 4)
                break;
            payloadLen = (_tcpBuffer[2] << 8) | _tcpBuffer[3];
            headerLen = 4;
        }
        else if (lenByte == 255)
        {
            if (_tcpBuffer.size() < 10)
                break;
            payloadLen = 0;
            for (int i = 0; i < 8; ++i)
                payloadLen = (payloadLen << 8) | (uint8_t)_tcpBuffer[2 + i];
            headerLen = 10;
        }

        if (_tcpBuffer.size() < headerLen + payloadLen)
            break;

        std::string payload = _tcpBuffer.substr(headerLen, payloadLen);

        _tcpBuffer.erase(0, headerLen + payloadLen);

        auto it = _handlers.find(opcode);
        if (it != _handlers.end())
            it->second(payload, opcode);
        else
            std::cerr << "[RECEIVER] No handler for opcode " << (int)opcode << std::endl;
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
        it->second(payload, opcode);
    }
    else
    {
        std::cerr << "[RECEIVER] No handler registered for opcode " << std::to_string(opcode) << std::endl;
    }
}
