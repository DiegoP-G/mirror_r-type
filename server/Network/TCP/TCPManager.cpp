#include "TCPManager.hpp"
#include "../../../transferData/opcode.hpp"
#include "../../../transferData/transferData.hpp"
#include "../Client.hpp"
#include "../NetworkManager.hpp"
#include <chrono>
#include <cstddef>
#include <iostream>
#include <stdexcept>
#include <string>
#include <sys/poll.h>
#include <unistd.h>

TCPManager::TCPManager(NetworkManager &ref) : _networkManagerRef(ref)
{
    _listenFd = socket(AF_INET, SOCK_STREAM, 0);
    if (_listenFd < 0)
        throw std::runtime_error("TCP socket failed");

    int opt = 1;
    if (setsockopt(_listenFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
        throw std::runtime_error("setsockopt(SO_REUSEADDR) failed");

    _addr.sin_family = AF_INET;
    _addr.sin_addr.s_addr = INADDR_ANY;
    _addr.sin_port = htons(SERVER_PORT);

    if (bind(_listenFd, (sockaddr *)&_addr, sizeof(_addr)) < 0)
        throw std::runtime_error("TCP bind failed");
    if (listen(_listenFd, 16) < 0)
        throw std::runtime_error("TCP listen failed");

    _pollFds.push_back({_listenFd, POLLIN, 0});
}

TCPManager::~TCPManager()
{
    close(_listenFd);
}

void TCPManager::acceptConnection()
{
    int addrlen = sizeof(_addr);

    int cfd = accept(_listenFd, (struct sockaddr *)&_addr, (socklen_t *)&addrlen);
    if (cfd < 0)
    {
        perror("accept failed");
    }
    else
    {
        _pollFds.push_back({cfd, POLLIN, 0});
        _networkManagerRef.getClientManager().addClient(Client("caca", cfd));
        std::cout << "[TCP] New client " << cfd << "\n";
        int code_udp = rand();
        _networkManagerRef.getClientManager().getClient(cfd)->setCodeUDP(code_udp);
        std::cout << "[TCP] TCP sent UDP code: " << std::to_string(code_udp) << "to caca \n";
        _networkManagerRef.addNewPlayer(cfd);
        sendFrameTCP(cfd, OPCODE_CODE_UDP, serializeInt(code_udp));
        std::cout << "h" << std::endl;
        // sleep(1);
        sendFrameTCP(cfd, OPCODE_WORLD_UPDATE, "sss");
    }
}

void TCPManager::handlePollin(size_t &i, pollfd &pfd)
{
    if (pfd.revents & POLLIN)
    {
        auto [opcode, payload] =
            receiveFrameTCP(pfd.fd, _networkManagerRef.getClientManager().getClientsMap()[pfd.fd].getBuffer());

        if (opcode == OPCODE_CLOSE_CONNECTION)
        {
            std::cout << "[TCP] Client " << pfd.fd << " disconnected\n";
            _networkManagerRef.getClientManager().removeClient(pfd.fd);
            _pollFds.erase(_pollFds.begin() + i);
            --i;
        }
        else
        {
            std::cout << "[TCP] Received: " << payload << "\n";
        }
    }
}

void TCPManager::update()
{
    int ret = poll(_pollFds.data(), _pollFds.size(), 100);
    if (ret < 0)
        throw std::runtime_error("TCP poll failed");

    for (size_t i = 0; i < _pollFds.size(); i++)
    {
        pollfd &pfd = _pollFds[i];

        if (pfd.fd == _listenFd && (pfd.revents & POLLIN))
        {
            acceptConnection();
        }
        else if (pfd.fd != _listenFd)
        {
            handlePollin(i, pfd);
        }
    }
}