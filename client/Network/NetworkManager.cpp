#include "NetworkManager.hpp"
#include "../../transferData/opcode.hpp"
#include "../../transferData/transferData.hpp" // ! include receiveFrameTCP
#include "../NetworkECSMediator.hpp"
#include "Receiver.hpp"
#include "Sender.hpp"

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
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

#include <cstring>
#include <iostream>

NetworkManager::NetworkManager(NetworkECSMediator &med, Sender &sender, Receiver &receiver)
    : _med(med), _sender(sender), _receiver(receiver), _tcpSocket(-1), _udpSocket(-1), _shouldStop(false),
      _serverPubKey(nullptr)
{
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
        std::cerr << "WSAStartup failed\n";
#endif
}

NetworkManager::~NetworkManager()
{
    if (_tcpSocket != -1)
    {
#ifdef _WIN32
        closesocket(_tcpSocket);
#else
        close(_tcpSocket);
#endif
        _tcpSocket = -1;
    }
    if (_udpSocket != -1)
    {
#ifdef _WIN32
        closesocket(_udpSocket);
#else
        close(_udpSocket);
#endif
        _udpSocket = -1;
    }
#ifdef _WIN32
    WSACleanup();
#endif
}

bool NetworkManager::setup(const char *serverIp, int port)
{
    _serverAddr = {};
    _serverAddr.sin_family = AF_INET;
    _serverAddr.sin_port = htons(port);
    if (inet_pton(AF_INET, serverIp, &_serverAddr.sin_addr) <= 0)
    {
        perror("inet_pton");
        return false;
    }

    _tcpSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (_tcpSocket < 0)
    {
        perror("TCP socket");
        return false;
    }

    if (connect(_tcpSocket, (sockaddr *)&_serverAddr, sizeof(_serverAddr)) < 0)
    {
        perror("TCP connect");
        return false;
    }

    int nodelay = 1;
    setsockopt(_tcpSocket, IPPROTO_TCP, TCP_NODELAY, (const char *)&nodelay, sizeof(nodelay));

    int bufsize = 262144;
    setsockopt(_tcpSocket, SOL_SOCKET, SO_SNDBUF, &bufsize, sizeof(bufsize));
    setsockopt(_tcpSocket, SOL_SOCKET, SO_RCVBUF, &bufsize, sizeof(bufsize));

    _udpSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (_udpSocket < 0)
    {
        perror("UDP socket");
        return false;
    }

    // UDP can be "connected" to default server address for send/recv
    connect(_udpSocket, (sockaddr *)&_serverAddr, sizeof(_serverAddr));

    _receiver.setServerAddr(_serverAddr);
    _receiver.setTcpSocket(_tcpSocket);
    _receiver.setUdpSocket(_udpSocket);

    _sender.setServerAddr(_serverAddr);
    _sender.setTcpSocket(_tcpSocket);
    _sender.setUdpSocket(_udpSocket);

    std::cout << "NetworkManager setup complete. Connected to " << serverIp << ":" << port << " (TCP & UDP, blocking)"
              << std::endl;
    return true;
}

void NetworkManager::loop()
{
    auto tcpLoop = [this]() {
        while (!_shouldStop.load() && _tcpSocket != -1)
        {
            _receiver.receiveTCPMessage();
        }
    };

    auto udpLoop = [this]() {
        while (!_shouldStop.load() && _udpSocket != -1)
        {
            _receiver.receiveUDPMessage();
        }
    };

    _tcpThread = std::thread(tcpLoop);
    _udpThread = std::thread(udpLoop);

    if (_tcpThread.joinable())
        _tcpThread.join();
    if (_udpThread.joinable())
        _udpThread.join();

    std::cout << "NetworkManager loop terminated" << std::endl;
}

void NetworkManager::stop()
{
    _shouldStop.store(true);
}

Sender &NetworkManager::getSender()
{
    return _sender;
}

Receiver &NetworkManager::getReceiver()
{
    return _receiver;
}
