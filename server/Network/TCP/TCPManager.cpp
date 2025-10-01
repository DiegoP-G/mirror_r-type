#include "TCPManager.hpp"
#include "../../../transferData/opcode.hpp"
#include "../../../transferData/transferData.hpp"
#include "../Client.hpp"
#include "../NetworkManager.hpp"
#include <cerrno>
#include <chrono>
#include <cstddef>
#include <cstring>
#include <fcntl.h>
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
    int flags = fcntl(_listenFd, F_GETFL, 0);
    fcntl(_listenFd, F_SETFL, flags | O_NONBLOCK);

    int opt = 1;
    if (setsockopt(_listenFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
        throw std::runtime_error("setsockopt(SO_REUSEADDR) failed");

    _addr.sin_family = AF_INET;
    _addr.sin_addr.s_addr = INADDR_ANY;
    _addr.sin_port = htons(SERVER_PORT);

    // Juste après socket() ou connect()

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
        return;
    }
    int flags = fcntl(cfd, F_GETFL, 0);
    fcntl(cfd, F_SETFL, flags | O_NONBLOCK);

    // Ajouter le client avec POLLIN (POLLOUT sera ajouté si besoin)
    _pollFds.push_back({cfd, POLLIN, 0});
    _networkManagerRef.getClientManager().addClient(Client("caca", cfd));

    std::cout << "[TCP] New client " << cfd << "\n";

    int code_udp = rand();
    _networkManagerRef.getClientManager().getClient(cfd)->setCodeUDP(code_udp);
    std::cout << "[TCP] TCP will send UDP code: " << code_udp << " to client " << cfd << "\n";

    _networkManagerRef.addNewPlayer(cfd);

    // Mettre les messages en file d'attente au lieu de les envoyer immédiatement
    queueMessage(cfd, OPCODE_CODE_UDP, serializeInt(code_udp));
    queueMessage(cfd, OPCODE_PLAYER_ID, serializeInt(cfd));

    // Activer POLLOUT pour ce client pour envoyer les messages
    setPollOut(cfd, true);
}

void TCPManager::queueMessage(int fd, uint8_t opcode, const std::string &payload)
{
    _pendingMessages[fd].push({opcode, payload});
    std::cout << "[TCP] Queued message (opcode: " << static_cast<int>(opcode) << ") for client " << fd << "\n";
}

void TCPManager::setPollOut(int fd, bool enable)
{
    for (auto &pfd : _pollFds)
    {
        if (pfd.fd == fd)
        {
            if (enable)
            {
                pfd.events |= POLLOUT;
                std::cout << "[TCP] Enabled POLLOUT for fd " << fd << "\n";
            }
            else
            {
                pfd.events &= ~POLLOUT;
                std::cout << "[TCP] Disabled POLLOUT for fd " << fd << "\n";
            }
            break;
        }
    }
}

void TCPManager::handlePollout(size_t i, pollfd &pfd)
{
    if (pfd.revents & POLLOUT)
    {
        // Vérifier s'il y a des messages en attente pour ce client
        if (_pendingMessages.count(pfd.fd) && !_pendingMessages[pfd.fd].empty())
        {
            auto [opcode, payload] = _pendingMessages[pfd.fd].front();

            // Essayer d'envoyer le message
            if (sendFrameTCP(pfd.fd, opcode, payload))
            {
                // Envoi réussi, retirer le message de la queue
                _pendingMessages[pfd.fd].pop();
                std::cout << "[TCP] Message sent, " << _pendingMessages[pfd.fd].size() << " remaining in queue for fd "
                          << pfd.fd << "\n";
            }
            else
            {
                // Envoi échoué (buffer plein), on réessaiera au prochain poll
                std::cout << "[TCP] Failed to send, will retry later\n";
                return;
            }

            // Si plus de messages en attente, désactiver POLLOUT
            if (_pendingMessages[pfd.fd].empty())
            {
                setPollOut(pfd.fd, false);
                // Nettoyer la map si la queue est vide
                _pendingMessages.erase(pfd.fd);
            }
        }
        else
        {
            // Pas de messages en attente, désactiver POLLOUT
            setPollOut(pfd.fd, false);
        }
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

            // Nettoyer les messages en attente pour ce client
            _pendingMessages.erase(pfd.fd);

            _networkManagerRef.getClientManager().removeClient(pfd.fd);
            _pollFds.erase(_pollFds.begin() + i);
            --i;
        }
        else
        {
            _networkManagerRef.getGameMediator().notify(static_cast<GameMediatorEvent>(opcode), payload);
            std::cout << "[TCP] Received: " << static_cast<int>(opcode) << " payload: " << payload << "\n";
        }
    }
}

void TCPManager::update()
{
    int ret = poll(_pollFds.data(), _pollFds.size(), 0);
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
            // Gérer POLLOUT en premier pour envoyer les messages en attente
            handlePollout(i, pfd);

            // Puis gérer POLLIN pour recevoir les données
            handlePollin(i, pfd);
        }
    }
}