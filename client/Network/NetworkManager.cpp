#include "NetworkManager.hpp"
#include "../../transferData/opcode.hpp"
#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <unistd.h>

NetworkManager::NetworkManager(ClientGame &ref)
    : _clientGameRef(ref), _sender(*this), _receiver(*this), _tcpSocket(-1), _udpSocket(-1)
{
}

NetworkManager::~NetworkManager()
{
    if (_tcpSocket != -1)
        close(_tcpSocket);
    if (_udpSocket != -1)
        close(_udpSocket);
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

    _udpSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (_udpSocket < 0)
    {
        perror("UDP socket");
        return false;
    }

    if (connect(_udpSocket, (sockaddr *)&_serverAddr, sizeof(_serverAddr)) < 0)
    {
        perror("UDP connect");
        return false;
    }

    _receiver.setServerAddr(_serverAddr);
    _receiver.setTcpSocket(_tcpSocket);
    _receiver.setUdpSocket(_udpSocket);

    _pollFds.clear();
    _pollFds.push_back({_tcpSocket, POLLIN, 0});
    _pollFds.push_back({_udpSocket, POLLIN, 0});

    std::cout << "NetworkManager setup complete. Connected to " << serverIp << ":" << port << " (TCP & UDP)"
              << std::endl;
    return true;
}

void NetworkManager::loop()
{
    constexpr int TIMEOUT = 1000;

    while (true)
    {
        int ret = poll(_pollFds.data(), _pollFds.size(), TIMEOUT);
        if (ret < 0)
        {
            perror("poll");
            break;
        }
        if (ret == 0)
        {
            continue;
        }

        for (auto &pfd : _pollFds)
        {
            if (pfd.revents & POLLIN)
            {
                if (pfd.fd == _tcpSocket)
                {
                    _receiver.receiveTCPMessage();
                }
                else if (pfd.fd == _udpSocket)
                {
                    _receiver.receiveUDPMessage();
                }
            }
        }
    }
}

Sender &NetworkManager::getSender()
{
    return _sender;
}

Receiver &NetworkManager::getReceiver()
{
    return _receiver;
}
