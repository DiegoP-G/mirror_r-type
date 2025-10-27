#include "TCPManager.hpp"
#include "../../../transferData/opcode.hpp"
#include "../../../transferData/transferData.hpp"
#include "../Client.hpp"
#include "../NetworkManager.hpp"
#include "transferData/hashUtils.hpp"
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
#define write(fd, buf, len) send(fd, (const char *)(buf), len, 0)
#define read(fd, buf, len) recv(fd, (char *)(buf), len, 0)
#define perror(msg)                                                                                                    \
    {                                                                                                                  \
        int err = WSAGetLastError();                                                                                   \
        std::cerr << msg << ": " << err << std::endl;                                                                  \
    }
#define close(fd) closesocket(fd)
typedef WSAPOLLFD pollfd;
#else
#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

TCPManager::TCPManager(NetworkManager &ref, PrometheusServer &metrics) : _networkManagerRef(ref), _metrics(metrics)
{
    _listenFd = socket(AF_INET, SOCK_STREAM, 0);
#ifdef _WIN32
    if (_listenFd == INVALID_SOCKET)
#else
    if (_listenFd < 0)
#endif
        throw std::runtime_error("TCP socket failed");

#ifdef _WIN32
    u_long mode = 1;
    ioctlsocket(_listenFd, FIONBIO, &mode);
#else
    int flags = fcntl(_listenFd, F_GETFL, 0);
    fcntl(_listenFd, F_SETFL, flags | O_NONBLOCK);
#endif

    int opt = 1;
#ifdef _WIN32
    setsockopt(_listenFd, SOL_SOCKET, SO_REUSEADDR, (const char *)&opt, sizeof(opt));
#else
    setsockopt(_listenFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
#endif

    _addr.sin_family = AF_INET;
    _addr.sin_addr.s_addr = INADDR_ANY;
    _addr.sin_port = htons(SERVER_PORT);

    if (bind(_listenFd, (sockaddr *)&_addr, sizeof(_addr)) < 0)
        throw std::runtime_error("TCP bind failed");

    if (listen(_listenFd, 16) < 0)
        throw std::runtime_error("TCP listen failed");

    _pollFds.push_back({_listenFd, POLLRDNORM | POLLRDBAND, 0});
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

void TCPManager::sendAESKey(int clientFd)
{
    std::optional<std::vector<uint8_t>> optionalKeyBytes =
        extractPEMBytesFromRSAKeyPair(_networkManagerRef.getServerPubKey());

    if (!optionalKeyBytes.has_value())
    {
        std::cerr << "Server failed to extract public key from RSA keypair";
        return;
    }
    std::vector<uint8_t> keyBytes = *optionalKeyBytes;
    std::string serverPubKeyStr(keyBytes.begin(), keyBytes.end());
    _networkManagerRef.getGameMediator().getNetworkManager().getTCPManager().sendMessage(
        clientFd, OPCODE_SERVER_PUB_KEY, serverPubKeyStr);
}

void TCPManager::handleNewConnection()
{
    sockaddr_in client_addr;
    socklen_t addrlen = sizeof(client_addr);

#ifdef _WIN32
    SOCKET cfd = accept(_listenFd, (struct sockaddr *)&client_addr, &addrlen);
    if (cfd == INVALID_SOCKET)
    {
        int error = WSAGetLastError();
        if (error != WSAEWOULDBLOCK)
            std::cerr << "[TCP] accept failed: " << error << std::endl;
        return;
    }
#else
    int cfd = accept(_listenFd, (struct sockaddr *)&client_addr, &addrlen);
    if (cfd < 0)
    {
        if (errno != EAGAIN && errno != EWOULDBLOCK)
            perror("[TCP] accept failed");
        return;
    }
#endif

#ifdef _WIN32
    u_long mode = 1;
    ioctlsocket(cfd, FIONBIO, &mode);
#else
    int flags = fcntl(cfd, F_GETFL, 0);
    fcntl(cfd, F_SETFL, flags | O_NONBLOCK);
#endif

    _pollFds.push_back({cfd, POLLRDNORM | POLLRDBAND, 0});
    _networkManagerRef.getClientManager().addClient(Client("client", cfd));
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

    int code_udp = rand();
    _networkManagerRef.getClientManager().getClient(cfd)->setCodeUDP(code_udp);

    sendMessage(cfd, OPCODE_PLAYER_ID, serializeInt(cfd));
    sendMessage(cfd, OPCODE_CODE_UDP, serializeInt(code_udp));
    sendAESKey(cfd);

    _networkManagerRef.addNewPlayer(cfd);
}

void TCPManager::sendMessage(int fd, uint8_t opcode, const std::string &payload)
{
    std::vector<uint8_t> frame;
    frame.push_back(opcode);

    size_t payloadLen = payload.size();
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

    frame.insert(frame.end(), payload.begin(), payload.end());
    _writeBuffers[fd].append(reinterpret_cast<char *>(frame.data()), frame.size());

    for (auto &pfd : _pollFds)
    {
        if (pfd.fd == fd)
        {
            pfd.events |= POLLWRNORM;
            break;
        }
    }

    _metrics.IncrementTCPSent();
    _metrics.AddTCPBytes(frame.size());
}

void TCPManager::handleClientWrite(int fd)
{
    if (_writeBuffers[fd].empty())
    {
        for (auto &pfd : _pollFds)
        {
            if (pfd.fd == fd)
            {
                pfd.events &= ~POLLWRNORM;
                break;
            }
        }
        return;
    }

    const char *data = _writeBuffers[fd].data();
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
            return;
        perror("[TCP] write error");
        return;
    }

    _writeBuffers[fd].erase(0, sent);
    if (_writeBuffers[fd].empty())
    {
        for (auto &pfd : _pollFds)
        {
            if (pfd.fd == fd)
            {
                pfd.events &= ~POLLWRNORM;
                break;
            }
        }
    }
}

void TCPManager::handleClientRead(int fd, size_t &index)
{
    std::string &readBuffer = _networkManagerRef.getClientManager().getClientsMap()[fd].getBuffer();

    while (true)
    {
        auto [opcode, payload] = receiveFrameTCP(fd, readBuffer);

        if (opcode == OPCODE_INCOMPLETE_DATA)
            break;

        if (opcode != OPCODE_INCOMPLETE_DATA && opcode != OPCODE_CLOSE_CONNECTION)
        {
            _metrics.IncrementTCPReceived();
            _metrics.AddTCPBytes(payload.size() + 2); // opcode + length
        }

        if (opcode == OPCODE_CLOSE_CONNECTION)
        {
            _writeBuffers.erase(fd);
            _networkManagerRef.getClientManager().removeClient(fd);
            _networkManagerRef.getGameMediator().notify(GameMediatorEvent::PlayerDisconnected, "", "", fd);
            _pollFds.erase(_pollFds.begin() + index);
            --index;
            return;
        }

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
        return;

    for (size_t i = 0; i < _pollFds.size(); i++)
    {
        pollfd &pfd = _pollFds[i];

        if (pfd.fd == _listenFd)
        {
            if (pfd.revents & (POLLRDNORM | POLLRDBAND))
                handleNewConnection();
            continue;
        }

        int fd = pfd.fd;

        if (pfd.revents & POLLWRNORM)
            handleClientWrite(fd);

        if (pfd.revents & (POLLRDNORM | POLLRDBAND))
            handleClientRead(fd, i);

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
