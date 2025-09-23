#include "NetworkManager.hpp"
#include <iostream>
#include <unistd.h>
#include <cstring>
#include <arpa/inet.h>

NetworkManager::NetworkManager()
    : _tcpSocket(-1), _udpSocket(-1) {}

NetworkManager::~NetworkManager() {
    if (_tcpSocket != -1) close(_tcpSocket);
    if (_udpSocket != -1) close(_udpSocket);
}

bool NetworkManager::setup(const char* serverIp, int port) {
    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    if (inet_pton(AF_INET, serverIp, &serverAddr.sin_addr) <= 0) {
        perror("inet_pton");
        return false;
    }

    _tcpSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (_tcpSocket < 0) {
        perror("TCP socket");
        return false;
    }

    if (connect(_tcpSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("TCP connect");
        return false;
    }

    _udpSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (_udpSocket < 0) {
        perror("UDP socket");
        return false;
    }

    if (connect(_udpSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("UDP connect");
        return false;
    }

    _pollFds.clear();
    _pollFds.push_back({_tcpSocket, POLLIN, 0});
    _pollFds.push_back({_udpSocket, POLLIN, 0});

    std::cout << "NetworkManager setup complete. Connected to " 
              << serverIp << ":" << port << " (TCP & UDP)" << std::endl;
    return true;
}

void NetworkManager::loop() {
    constexpr int TIMEOUT = 1000; // ms

    while (true) {
        int ret = poll(_pollFds.data(), _pollFds.size(), TIMEOUT);
        if (ret < 0) {
            perror("poll");
            break;
        }
        if (ret == 0) {
            continue;
        }

        for (auto &pfd : _pollFds) {
            if (pfd.revents & POLLIN) {
                if (pfd.fd == _tcpSocket) {
                    char buffer[1024];
                    ssize_t n = recv(_tcpSocket, buffer, sizeof(buffer), 0);
                    if (n > 0) {
                        std::cout << "message tcp: " << buffer << std::endl;
                        // _receiver.handleTCP(buffer, n);
                    } else if (n == 0) {
                        std::cerr << "Server closed TCP connection" << std::endl;
                        close(_tcpSocket);
                        _tcpSocket = -1;
                        return;
                    }
                } else if (pfd.fd == _udpSocket) {
                    char buffer[1024];
                    ssize_t n = recv(_udpSocket, buffer, sizeof(buffer), 0);
                    if (n > 0) {
                        std::cout << "message udp: " << buffer << std::endl;
                        // _receiver.handleUDP(buffer, n);
                    }
                }
            }
        }
    }
}

Sender& NetworkManager::getSender() {
    return _sender;
}

Receiver& NetworkManager::getReceiver() {
    return _receiver;
}
