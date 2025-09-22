#include "NetworkManager.hpp"
#include "ClientManager.hpp"
#include "NetworkMediator.hpp"
#include <algorithm>
#include <arpa/inet.h>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <memory>
#include <netinet/in.h>
#include <poll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

NetworkManager::NetworkManager() : _tcpSocket(-1), _udpSocket(-1), _networkMediator(*this)
{
}

NetworkManager::~NetworkManager()
{
    if (_tcpSocket != -1)
        close(_tcpSocket);
    if (_udpSocket != -1)
        close(_udpSocket);
}

void NetworkManager::setupPolls()
{
    if (_tcpSocket != -1)
    {
        pollfd pfd{};
        pfd.fd = _tcpSocket;
        pfd.events = POLLIN;
        _fds.push_back(pfd);
    }

    if (_udpSocket != -1)
    {
        pollfd pfd{};
        pfd.fd = _udpSocket;
        pfd.events = POLLIN;
        _fds.push_back(pfd);
    }

    for (auto &[sock, client] : _clientManager.getClientsMap())
    {
        pollfd pfd{};
        pfd.fd = sock;
        pfd.events = POLLIN;
        _fds.push_back(pfd);
    }
    std::cout << " setup sockets" << std::endl;
}

void NetworkManager::setupSockets(int port)
{
    _tcpSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (_tcpSocket < 0)
    {
        std::cerr << "Failed to create TCP socket\n";
        return;
    }

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);

    if (bind(_tcpSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
    {
        std::cerr << "Bind failed\n";
        return;
    }

    if (listen(_tcpSocket, 5) < 0)
    {
        std::cerr << "Listen failed\n";
        return; // throw
    }

    _udpSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (_udpSocket < 0)
    {
        std::cerr << "Failed to create UDP socket\n";
        return;
    }

    if (bind(_udpSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
    {
        std::cerr << "UDP bind failed\n";
        return;
    }

    fcntl(_udpSocket, F_SETFL, O_NONBLOCK);

    std::cout << "Server listening on port " << port << " (TCP & UDP)\n";
}

void NetworkManager::acceptClients()
{
    sockaddr_in clientAddr{};
    socklen_t clientLen = sizeof(clientAddr);
    int clientSock = accept(_tcpSocket, (struct sockaddr *)&clientAddr, &clientLen);
    std::cout << "c" << std::endl;

    if (clientSock < 0)
    {
        perror("accept failed");
        return;
    }

    std::cout << clientSock << std::endl;
    Client newClient("Player", clientSock);
    std::cout << "c" << std::endl;
    newClient.setConnected(true);
    std::cout << "c" << std::endl;
    _clientManager.addClient(newClient);
    std::cout << "c" << std::endl;
    std::cout << "c" << std::endl;

    pollfd pfd{};
    pfd.fd = clientSock;
    pfd.events = POLLIN;
    _fds.push_back(pfd);
    std::cout << "c" << std::endl;

    std::cout << "Accepted TCP client, socket: " << clientSock << "\n";
}

void NetworkManager::receiveData()
{
    char buffer[1024];

    auto &clients = _clientManager.getClientsMap();
    for (auto &[sock, client] : clients)
    {
        int bytesReceived = recv(sock, buffer, sizeof(buffer) - 1, MSG_DONTWAIT);
        if (bytesReceived > 0)
        {
            buffer[bytesReceived] = '\0';
            std::string msg(buffer);
            std::cout << "received TCP: " << msg << std::endl;
        }
    }

    sockaddr_in udpAddr{};
    socklen_t addrLen = sizeof(udpAddr);
    int bytesReceived = recvfrom(_udpSocket, buffer, sizeof(buffer) - 1, 0, (struct sockaddr *)&udpAddr, &addrLen);
    if (bytesReceived > 0)
    {
        buffer[bytesReceived] = '\0';
        std::string msg(buffer);
        std::cout << "received UDP: " << msg << std::endl;
    }
}

void NetworkManager::disconnectClient(int clientSocket)
{
    _clientManager.removeClient(clientSocket);
    close(clientSocket);

    // remove from _fds
    _fds.erase(
        std::remove_if(_fds.begin(), _fds.end(), [clientSocket](const pollfd &pfd) { return pfd.fd == clientSocket; }),
        _fds.end());

    std::cout << "Disconnected client: " << clientSocket << "\n";
}

void NetworkManager::pollOnce()
{
    int ret = poll(_fds.data(), _fds.size(), 1000);

    if (ret < 0)
    {
        std::cerr << "poll() failed\n";
    }
    if (ret == 0)
    {
        std::cout << "0" << std::endl;
        return;
    }

    for (auto &pfd : _fds)
    {
        if (pfd.revents & POLLIN)
        {
            std::cout << "x" << std::endl;
            if (pfd.fd == _tcpSocket)
            {
                acceptClients();
            }
            // else
            // {
            //     receiveData();
            // }
        }
    }
}
