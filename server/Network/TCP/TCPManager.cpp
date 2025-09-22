#include "TCPManager.hpp"
#include "../Client.hpp"
#include "../NetworkManager.hpp"
#include <iostream>
#include <stdexcept>
#include "../../transferData/transferData.hpp"

TCPManager::TCPManager(NetworkManager &ref) : _networkManagerRef(ref)
{
    _listenFd = socket(AF_INET, SOCK_STREAM, 0);
    if (_listenFd < 0)
        throw std::runtime_error("TCP socket failed");

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(SERVER_PORT);

    if (bind(_listenFd, (sockaddr *)&addr, sizeof(addr)) < 0)
        throw std::runtime_error("TCP bind failed");
    if (listen(_listenFd, 16) < 0)
        throw std::runtime_error("TCP listen failed");

    _pollFds.push_back({_listenFd, POLLIN, 0});
}

TCPManager::~TCPManager()
{
    close(_listenFd);
}

void TCPManager::update()
{
    int ret = poll(_pollFds.data(), _pollFds.size(), 0);
    if (ret < 0)
        throw std::runtime_error("TCP poll failed");

    for (size_t i = 0; i < _pollFds.size(); i++)
    {
        auto &pfd = _pollFds[i];

        if (pfd.fd == _listenFd && (pfd.revents & POLLIN))
        {
            int cfd = accept(_listenFd, nullptr, nullptr);
            if (cfd >= 0)
            {
                _pollFds.push_back({cfd, POLLIN, 0});
                Client newClient = Client("caca", cfd);
                _networkManagerRef.getClientManager().addClient(newClient);
                std::cout << "[TCP] New client " << cfd << "\n";
            }
        }
        else if (pfd.fd != _listenFd && (pfd.revents & POLLIN))
        {
            auto [opcode, payload] =
                receiveFrameTCP(pfd.fd, _networkManagerRef.getClientManager().getClientsMap()[pfd.fd].getBuffer());
            char buf[1024];
            ssize_t n = recv(pfd.fd, buf, sizeof(buf), 0);
            if (n <= 0)
            {
                std::cout << "[TCP] Client " << pfd.fd << " disconnected\n";
                _networkManagerRef.getClientManager().removeClient(pfd.fd);
                _pollFds.erase(_pollFds.begin() + i);
                --i;
            }
            else

            {
                // _clients[pfd.fd].append(buf, n);
                // std::cout << "[TCP] Received: " << _clients[pfd.fd] << "\n";
            }
        }
    }
}

void TCPManager::sendToClient(int fd, const std::string &msg)
{
    send(fd, msg.data(), msg.size(), 0);
}