#include "TCPManager.hpp"
#include "../../../transferData/opcode.hpp"
#include "../../../transferData/transferData.hpp"
#include "../Client.hpp"
#include "../NetworkManager.hpp"
#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <stdexcept>
#include <sys/poll.h>
#include <unistd.h>

TCPManager::TCPManager(NetworkManager &ref) : _networkManagerRef(ref)
{
    // Créer le socket d'écoute
    _listenFd = socket(AF_INET, SOCK_STREAM, 0);
    if (_listenFd < 0)
        throw std::runtime_error("TCP socket failed");

    // Non-bloquant
    int flags = fcntl(_listenFd, F_GETFL, 0);
    fcntl(_listenFd, F_SETFL, flags | O_NONBLOCK);

    // Réutiliser l'adresse
    int opt = 1;
    setsockopt(_listenFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // Configuration de l'adresse
    _addr.sin_family = AF_INET;
    _addr.sin_addr.s_addr = INADDR_ANY;
    _addr.sin_port = htons(SERVER_PORT);

    // Bind et listen
    if (bind(_listenFd, (sockaddr *)&_addr, sizeof(_addr)) < 0)
        throw std::runtime_error("TCP bind failed");

    if (listen(_listenFd, 16) < 0)
        throw std::runtime_error("TCP listen failed");

    // Ajouter au poll (seulement POLLIN pour le listen socket)
    _pollFds.push_back({_listenFd, POLLIN, 0});

    std::cout << "[TCP] Server listening on port " << SERVER_PORT << std::endl;
}

TCPManager::~TCPManager()
{
    close(_listenFd);
}

void TCPManager::handleNewConnection()
{
    sockaddr_in client_addr;
    socklen_t addrlen = sizeof(client_addr);

    int cfd = accept(_listenFd, (struct sockaddr *)&client_addr, &addrlen);
    if (cfd < 0)
    {
        if (errno != EAGAIN && errno != EWOULDBLOCK)
            perror("[TCP] accept failed");
        return;
    }

    // Mettre en non-bloquant
    int flags = fcntl(cfd, F_GETFL, 0);
    fcntl(cfd, F_SETFL, flags | O_NONBLOCK);

    // Ajouter au poll (POLLIN seulement au début)
    _pollFds.push_back({cfd, POLLIN, 0});

    // Créer le client
    _networkManagerRef.getClientManager().addClient(Client("client", cfd));

    // Initialiser le buffer d'écriture vide
    _writeBuffers[cfd] = "";

    std::cout << "[TCP] Client " << cfd << " connected" << std::endl;

    // Générer le code UDP
    int code_udp = rand();
    _networkManagerRef.getClientManager().getClient(cfd)->setCodeUDP(code_udp);

    // Envoyer les messages de connexion
    sendMessage(cfd, OPCODE_PLAYER_ID, serializeInt(cfd));
    sendMessage(cfd, OPCODE_CODE_UDP, serializeInt(code_udp));

    // Notifier la création du joueur
    _networkManagerRef.addNewPlayer(cfd);
    _networkManagerRef.sendAllEntitiesToClient(cfd);
}

void TCPManager::sendMessage(int fd, uint8_t opcode, const std::string &payload)
{
    // Construire la trame
    std::vector<uint8_t> frame;
    frame.push_back(opcode);

    size_t payloadLen = payload.size();

    // Encoder la longueur
    if (payloadLen <= 253)
    {
        frame.push_back(static_cast<uint8_t>(payloadLen));
    }
    else if (payloadLen <= 65535)
    {
        frame.push_back(254);
        frame.push_back((payloadLen >> 8) & 0xFF);
        frame.push_back(payloadLen & 0xFF);
    }
    else
    {
        frame.push_back(255);
        for (int i = 7; i >= 0; --i)
            frame.push_back((payloadLen >> (8 * i)) & 0xFF);
    }

    // Ajouter le payload
    frame.insert(frame.end(), payload.begin(), payload.end());

    // Ajouter au buffer d'écriture
    _writeBuffers[fd].append(reinterpret_cast<char *>(frame.data()), frame.size());

    // std::cout << "[TCP] Queued message for client " << fd
    //           << " (opcode: 0x" << std::hex << (int)opcode << std::dec
    //           << ", " << payload.size() << " bytes payload, "
    //           << frame.size() << " bytes total)" << std::endl;

    // Activer POLLOUT pour ce socket
    for (auto &pfd : _pollFds)
    {
        if (pfd.fd == fd)
        {
            pfd.events |= POLLOUT;
            break;
        }
    }
}

void TCPManager::handleClientWrite(int fd)
{
    // Rien à envoyer
    if (_writeBuffers[fd].empty())
    {
        // Désactiver POLLOUT
        for (auto &pfd : _pollFds)
        {
            if (pfd.fd == fd)
            {
                pfd.events &= ~POLLOUT;
                break;
            }
        }
        return;
    }

    // Essayer d'envoyer ce qui reste dans le buffer
    const char *data = _writeBuffers[fd].data();
    size_t remaining = _writeBuffers[fd].size();

    ssize_t sent = write(fd, data, remaining);

    if (sent < 0)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
        {
            // Buffer plein, on réessaiera au prochain POLLOUT
            std::cout << "[TCP] Client " << fd << " write blocked (EAGAIN)" << std::endl;
            return;
        }

        // Erreur réelle
        perror("[TCP] write error");
        return;
    }

    // Retirer ce qui a été envoyé du buffer
    _writeBuffers[fd].erase(0, sent);

    std::cout << "[TCP] Sent " << sent << " bytes to client " << fd << ", remaining: " << _writeBuffers[fd].size()
              << " bytes" << std::endl;

    // Si tout est envoyé, désactiver POLLOUT
    if (_writeBuffers[fd].empty())
    {
        for (auto &pfd : _pollFds)
        {
            if (pfd.fd == fd)
            {
                pfd.events &= ~POLLOUT;
                std::cout << "[TCP] All data sent to client " << fd << ", disabling POLLOUT" << std::endl;
                break;
            }
        }
    }
}

void TCPManager::handleClientRead(int fd, size_t &index)
{
    // Récupérer le buffer de lecture du client
    std::string &readBuffer = _networkManagerRef.getClientManager().getClientsMap()[fd].getBuffer();

    // Lire les messages disponibles
    while (true)
    {
        auto [opcode, payload] = receiveFrameTCP(fd, readBuffer);

        if (opcode == OPCODE_INCOMPLETE_DATA)
        {
            // Données incomplètes, on attendra plus de données
            break;
        }

        if (opcode == OPCODE_CLOSE_CONNECTION)
        {
            std::cout << "[TCP] Client " << fd << " disconnected" << std::endl;

            // Nettoyer
            _writeBuffers.erase(fd);
            _networkManagerRef.getClientManager().removeClient(fd);
            _pollFds.erase(_pollFds.begin() + index);
            --index;
            return;
        }

        // Message valide reçu
        std::cout << "[TCP] Received from client " << fd << " (opcode: 0x" << std::hex << (int)opcode << std::dec
                  << ", " << payload.size() << " bytes)" << std::endl;

        // Notifier le médiateur
        _networkManagerRef.getGameMediator().notify(static_cast<GameMediatorEvent>(opcode), payload);
    }
}

void TCPManager::update()
{
    // Poll avec timeout 0 (non-bloquant)
    int ret = poll(_pollFds.data(), _pollFds.size(), 0);

    if (ret < 0)
    {
        if (errno != EINTR)
            throw std::runtime_error("TCP poll failed");
        return;
    }

    if (ret == 0)
        return; // Pas d'événements

    // Traiter les événements
    for (size_t i = 0; i < _pollFds.size(); i++)
    {
        pollfd &pfd = _pollFds[i];

        // Socket d'écoute
        if (pfd.fd == _listenFd)
        {
            if (pfd.revents & POLLIN)
                handleNewConnection();
            continue;
        }

        // Sockets clients
        int fd = pfd.fd;

        // Gérer l'écriture en premier (pour vider les buffers)
        if (pfd.revents & POLLOUT)
        {
            handleClientWrite(fd);
        }

        // Puis gérer la lecture
        if (pfd.revents & POLLIN)
        {
            handleClientRead(fd, i);
        }

        // Gérer les erreurs
        if (pfd.revents & (POLLERR | POLLHUP))
        {
            std::cout << "[TCP] Client " << fd << " error/hangup" << std::endl;
            _writeBuffers.erase(fd);
            _networkManagerRef.getClientManager().removeClient(fd);
            _pollFds.erase(_pollFds.begin() + i);
            --i;
        }
    }
}