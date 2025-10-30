#include "Sender.hpp"
#include "../../transferData/transferData.hpp"
#include "../NetworkECSMediator.hpp"
#include <cstdint>
#include <iostream>
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
#include <windows.h>
#else
#include <arpa/inet.h>
#include <netinet/in.h>
#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

void Sender::sendTcp(uint8_t opcode, const std::string &payload)
{
    if (_tcpSocket == -1)
    {
        std::cerr << "[Sender] TCP socket not set!" << std::endl;
        return;
    }

    sendFrameTCP(_tcpSocket, opcode, payload); // ! blocking write, handles partial sends internally
    std::cout << "[Sender] Sent TCP frame (opcode=" << (int)opcode << ", size=" << payload.size() << ")" << std::endl;
}

void Sender::sendUdp(uint8_t opcode, const std::string &payload)
{
    if (_udpSocket == -1)
    {
        std::cerr << "[Sender] UDP socket not set!" << std::endl;
        return;
    }

    sendFrameUDP(_udpSocket, opcode, payload, _serverAddr, sizeof(_serverAddr));
}
