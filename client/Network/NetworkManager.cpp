#include "NetworkManager.hpp"
#include "../../transferData/opcode.hpp"
#include "../NetworkECSMediator.hpp"
#include "Receiver.hpp"
#include "Sender.hpp"
#include <arpa/inet.h>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <netinet/tcp.h>
#include <unistd.h>

NetworkManager::NetworkManager(NetworkECSMediator &med, Sender &sender, Receiver &receiver)
    : _med(med), _sender(sender), _receiver(receiver), _tcpSocket(-1), _udpSocket(-1), _shouldStop(false),
    _serverPubKey(nullptr)
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
    int flags = fcntl(_tcpSocket, F_GETFL, 0);
    fcntl(_tcpSocket, F_SETFL, flags | O_NONBLOCK);
    int nodelay = 1;
    if (setsockopt(_tcpSocket, IPPROTO_TCP, TCP_NODELAY, &nodelay, sizeof(nodelay)) < 0)
    {
        perror("[TCP Client] Warning: Failed to set TCP_NODELAY");
    }

    int sndbuf = 262144; // 256 KB
    int rcvbuf = 262144; // 256 KB
    setsockopt(_tcpSocket, SOL_SOCKET, SO_SNDBUF, &sndbuf, sizeof(sndbuf));
    setsockopt(_tcpSocket, SOL_SOCKET, SO_RCVBUF, &rcvbuf, sizeof(rcvbuf));

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

    // _med.setSender(_sender);
    // med.setReceiver(_receiver);

    _receiver.setServerAddr(_serverAddr);
    _receiver.setTcpSocket(_tcpSocket);
    _receiver.setUdpSocket(_udpSocket);

    _sender.setServerAddr(_serverAddr);
    _sender.setTcpSocket(_tcpSocket);
    _sender.setUdpSocket(_udpSocket);

    _pollFds.clear();
    _pollFds.push_back({_tcpSocket, POLLIN, 0});
    _pollFds.push_back({_udpSocket, POLLIN, 0});

    std::cout << "NetworkManager setup complete. Connected to " << serverIp << ":" << port << " (TCP & UDP)"
              << std::endl;
    return true;
}

void NetworkManager::loop()
{
    constexpr int TIMEOUT = 100; // Timeout de 100ms pour vérifier périodiquement _shouldStop

    while (!_shouldStop.load())
    {
        int ret = poll(_pollFds.data(), _pollFds.size(), TIMEOUT);

        if (ret < 0)
        {
            if (errno == EINTR)
                continue;
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
                if (pfd.fd == _udpSocket)
                {
                    _receiver.receiveUDPMessage();
                }
                else if (pfd.fd == _tcpSocket)
                {
                    _receiver.receiveTCPMessage();
                }
            }
        }
    }

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