#include "TCPManager.hpp"
#include "../../../transferData/opcode.hpp"
#include "../../../transferData/transferData.hpp"
#include "../Client.hpp"
#include "../NetworkManager.hpp"
#include <cerrno>
#include <cstring>
#include <iostream>
#include <stdexcept>
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
#define ssize_t int
#define write(fd, buf, len) send(fd, (const char*)(buf), len, 0)
#define read(fd, buf, len) recv(fd, (char*)(buf), len, 0)
#define perror(msg) { int err = WSAGetLastError(); std::cerr << msg << ": " << err << std::endl; }
#define close(fd) closesocket(fd)
// DO NOT redefine POLLIN, POLLOUT, etc. - use Windows native values from winsock2.h
// DO NOT redefine poll - use WSAPoll directly
typedef WSAPOLLFD pollfd;
#else
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <poll.h>
#include <fcntl.h>
#endif

TCPManager::TCPManager(NetworkManager& ref) : _networkManagerRef(ref)
{
    // Créer le socket d'écoute
    _listenFd = socket(AF_INET, SOCK_STREAM, 0);
#ifdef _WIN32
    if (_listenFd == INVALID_SOCKET)
#else
    if (_listenFd < 0)
#endif
        throw std::runtime_error("TCP socket failed");

    // Non-bloquant
#ifdef _WIN32
    u_long mode = 1;
    ioctlsocket(_listenFd, FIONBIO, &mode);
#else
    int flags = fcntl(_listenFd, F_GETFL, 0);
    fcntl(_listenFd, F_SETFL, flags | O_NONBLOCK);
#endif

    // Réutiliser l'adresse
    int opt = 1;
#ifdef _WIN32
    setsockopt(_listenFd, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));
#else
    setsockopt(_listenFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
#endif

    // Configuration de l'adresse
    _addr.sin_family = AF_INET;
    _addr.sin_addr.s_addr = INADDR_ANY;
    _addr.sin_port = htons(SERVER_PORT);

    // Bind et listen
    if (bind(_listenFd, (sockaddr*)&_addr, sizeof(_addr)) < 0)
        throw std::runtime_error("TCP bind failed");

    if (listen(_listenFd, 16) < 0)
        throw std::runtime_error("TCP listen failed");

    // Ajouter au poll (seulement POLLIN pour le listen socket)
    _pollFds.push_back({ _listenFd, POLLRDNORM | POLLRDBAND, 0 });

    std::cout << "[TCP] Server listening on port " << SERVER_PORT << std::endl;
}

TCPManager::~TCPManager()
{
#ifdef _WIN32
    closesocket(_listenFd);
#else
    close(_listenFd);
#endif
}

void TCPManager::handleNewConnection()
{
    sockaddr_in client_addr;
    socklen_t addrlen = sizeof(client_addr);

#ifdef _WIN32
    SOCKET cfd = accept(_listenFd, (struct sockaddr*)&client_addr, &addrlen);
    if (cfd == INVALID_SOCKET)
    {
        int error = WSAGetLastError();
        if (error != WSAEWOULDBLOCK)
            std::cerr << "[TCP] accept failed: " << error << std::endl;
        return;
    }
#else
    int cfd = accept(_listenFd, (struct sockaddr*)&client_addr, &addrlen);
    if (cfd < 0)
    {
        if (errno != EAGAIN && errno != EWOULDBLOCK)
            perror("[TCP] accept failed");
        return;
    }
#endif

    // Mettre en non-bloquant
#ifdef _WIN32
    u_long mode = 1;
    ioctlsocket(cfd, FIONBIO, &mode);
#else
    int flags = fcntl(cfd, F_GETFL, 0);
    fcntl(cfd, F_SETFL, flags | O_NONBLOCK);
#endif

    // Ajouter au poll (POLLIN seulement au début)
    _pollFds.push_back({ cfd, POLLRDNORM | POLLRDBAND, 0 });

    // Créer le client
    _networkManagerRef.getClientManager().addClient(Client("client", cfd));

    // Initialiser le buffer d'écriture vide
    _writeBuffers[cfd] = "";

    if (_networkManagerRef.getClientManager().isBannedIP(inet_ntoa(client_addr.sin_addr)))
    {
        sendMessage(cfd, OPCODE_BAN_NOTIFICATION, "");
        handleClientWrite(cfd);

        _networkManagerRef.getClientManager().removeClient(cfd);
        _networkManagerRef.getClientManager().addAdminPanelLog(std::string("Banned client ") +
                                                               inet_ntoa(client_addr.sin_addr) + " tried to connect.");
        return;
    }

    std::cout << "[TCP] Client " << cfd << " connected" << std::endl;

    // Générer le code UDP
    int code_udp = rand();
    _networkManagerRef.getClientManager().getClient(cfd)->setCodeUDP(code_udp);

    // Envoyer les messages de connexion
    sendMessage(cfd, OPCODE_PLAYER_ID, serializeInt(cfd));
    sendMessage(cfd, OPCODE_CODE_UDP, serializeInt(code_udp));

    // Notifier la création du joueur
    _networkManagerRef.addNewPlayer(cfd);
}

void TCPManager::sendMessage(int fd, uint8_t opcode, const std::string& payload)
{
    // Construire la trame
    std::cout << "[TCP] sendMessage: fd=" << fd << ", opcode=" << static_cast<int>(opcode)
              << ", payloadLen=" << payload.size() << std::endl;
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

    std::cout << "finishing the frame"<<std::endl;
    // Ajouter au buffer d'écriture
    _writeBuffers[fd].append(reinterpret_cast<char*>(frame.data()), frame.size());
    std::cout << "Append to the buffer" << std::endl;
    // Activer POLLOUT pour ce socket
    for (auto& pfd : _pollFds)
    {
        std::cout << "pfd.fd" << pfd.fd << std::endl;
        if (pfd.fd == fd)
        {
            pfd.events |= POLLWRNORM;
            break;
        }
    }
    std::cout << "finishing" << std::endl;
}

void TCPManager::handleClientWrite(int fd)
{
    // Rien à envoyer
    if (_writeBuffers[fd].empty())
    {
        // Désactiver POLLOUT
        for (auto& pfd : _pollFds)
        {
            if (pfd.fd == fd)
            {
                pfd.events &= ~POLLWRNORM;
                break;
            }
        }
        return;
    }

    // Essayer d'envoyer ce qui reste dans le buffer
    const char* data = _writeBuffers[fd].data();
    size_t remaining = _writeBuffers[fd].size();

    ssize_t sent = write(fd, data, remaining);

    if (sent < 0)
    {
#ifdef _WIN32
        int error = WSAGetLastError();
        if (error == WSAEWOULDBLOCK)
#else
        if (errno == EAGAIN || errno == EWOULDBLOCK)
#endif
        {
            // Buffer plein, on réessaiera au prochain POLLOUT
            return;
        }

        // Erreur réelle
        perror("[TCP] write error");
        return;
    }

    // Retirer ce qui a été envoyé du buffer
    _writeBuffers[fd].erase(0, sent);

    // Si tout est envoyé, désactiver POLLOUT
    if (_writeBuffers[fd].empty())
    {
        for (auto& pfd : _pollFds)
        {
            if (pfd.fd == fd)
            {
                pfd.events &= ~POLLWRNORM;
                break;
            }
        }
    }
}

void TCPManager::handleClientRead(int fd, size_t& index)
{
    // Récupérer le buffer de lecture du client
    std::string& readBuffer = _networkManagerRef.getClientManager().getClientsMap()[fd].getBuffer();

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
            // Nettoyer
            _writeBuffers.erase(fd);

            std::cout << "Before removing the client" << std::endl;
            _networkManagerRef.getClientManager().removeClient(fd);
            std::cout << "Notifying the mediator" << std::endl;
            _networkManagerRef.getGameMediator().notify(GameMediatorEvent::PlayerDisconnected, "", "", fd);
            _pollFds.erase(_pollFds.begin() + index);
            --index;
            return;
        }

        // Message valide reçu
        // Notifier le médiateur
        _networkManagerRef.getGameMediator().notify(static_cast<GameMediatorEvent>(opcode), payload, "", fd);
    }
}

void TCPManager::update()
{
#ifdef _WIN32
    int ret = WSAPoll(_pollFds.data(), static_cast<ULONG>(_pollFds.size()), 0);
    if (ret == SOCKET_ERROR)
    {
        int error = WSAGetLastError();
        throw std::runtime_error("TCP poll failed with error: " + std::to_string(error));
    }
#else
    int ret = poll(_pollFds.data(), _pollFds.size(), 0);
    if (ret < 0)
    {
        if (errno != EINTR)
            throw std::runtime_error("TCP poll failed");
        return;
    }
#endif

    if (ret == 0)
        return; // Pas d'événements

    // Traiter les événements
    for (size_t i = 0; i < _pollFds.size(); i++)
    {
        pollfd& pfd = _pollFds[i];

        // Socket d'écoute
        if (pfd.fd == _listenFd)
        {
            if (pfd.revents & (POLLRDNORM | POLLRDBAND))
                handleNewConnection();
            continue;
        }

        // Sockets clients
        int fd = pfd.fd;

        // Gérer l'écriture en premier (pour vider les buffers)
        if (pfd.revents & POLLWRNORM)
        {
            handleClientWrite(fd);
        }

        // Puis gérer la lecture
        if (pfd.revents & (POLLRDNORM | POLLRDBAND))
        {
            handleClientRead(fd, i);
        }

        // Gérer les erreurs
        if (pfd.revents & (POLLERR | POLLHUP))
        {
            _writeBuffers.erase(fd);
            _networkManagerRef.getClientManager().removeClient(fd);
            _networkManagerRef.getGameMediator().notify(GameMediatorEvent::PlayerDisconnected, "", "", fd);
            _pollFds.erase(_pollFds.begin() + i);
            --i;
        }
    }
}