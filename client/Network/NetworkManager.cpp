#include "NetworkManager.hpp"
#include "../../transferData/opcode.hpp"
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
    #include <sys/socket.h>
    #include <netinet/tcp.h>
    #include <unistd.h>
#endif

#include <cstring>
#include <fcntl.h>
#include <iostream>

NetworkManager::NetworkManager(NetworkECSMediator &med, Sender &sender, Receiver &receiver)
    : _med(med), _sender(sender), _receiver(receiver), _tcpSocket(-1), _udpSocket(-1), _shouldStop(false)
{
    #ifdef _WIN32
        WSADATA wsaData;
    
        int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
        if (result != 0) {
            printf("WSAStartup failed: %d\n", result);
        }
    #endif
}

NetworkManager::~NetworkManager()
{


    if (_tcpSocket != -1) {
        #ifdef _WIN32
            closesocket(_tcpSocket);
            WSACleanup();

        #else
            close(_tcpSocket);
        #endif
    }
    if (_udpSocket != -1) {
        #ifdef _WIN32
            closesocket(_udpSocket);
            WSACleanup();

        #else
            close(_udpSocket);
        #endif
    }
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
    #ifdef _WIN32
        u_long mode = 1; // 1 = non-blocking
        if (ioctlsocket(_tcpSocket, FIONBIO, &mode) != 0) {
            printf("[TCP Client] Warning: Failed to set non-blocking mode\n");
        }
    #else
        int flags = fcntl(_tcpSocket, F_GETFL, 0);
        fcntl(_tcpSocket, F_SETFL, flags | O_NONBLOCK);
    #endif

        int nodelay = 1;
    #ifdef _WIN32
        if (setsockopt(_tcpSocket, IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<const char*>(&nodelay), sizeof(nodelay)) < 0) {
            printf("[TCP Client] Warning: Failed to set TCP_NODELAY\n");
        }
    #else
        if (setsockopt(_tcpSocket, IPPROTO_TCP, TCP_NODELAY, &nodelay, sizeof(nodelay)) < 0) {
            perror("[TCP Client] Warning: Failed to set TCP_NODELAY");
        }
    #endif

        // ---- Buffers ----
        int sndbuf = 262144; // 256 KB
        int rcvbuf = 262144; // 256 KB
    #ifdef _WIN32
        setsockopt(_tcpSocket, SOL_SOCKET, SO_SNDBUF, reinterpret_cast<const char*>(&sndbuf), sizeof(sndbuf));
        setsockopt(_tcpSocket, SOL_SOCKET, SO_RCVBUF, reinterpret_cast<const char*>(&rcvbuf), sizeof(rcvbuf));
    #else
        setsockopt(_tcpSocket, SOL_SOCKET, SO_SNDBUF, &sndbuf, sizeof(sndbuf));
        setsockopt(_tcpSocket, SOL_SOCKET, SO_RCVBUF, &rcvbuf, sizeof(rcvbuf));
    #endif
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
        #ifdef _WIN32
            int ret = WSAPoll(_pollFds.data(), _pollFds.size(), TIMEOUT);
        #else
            int ret = poll(_pollFds.data(), _pollFds.size(), TIMEOUT);
        #endif

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