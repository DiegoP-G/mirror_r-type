#include "UDPManager.hpp"
#include "../../../transferData/opcode.hpp"
#include "../../../transferData/transferData.hpp"
#include "../../Game/GameMediator.hpp"
#include "../NetworkManager.hpp"
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <string>

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
#define close(fd) closesocket(fd)
typedef WSAPOLLFD pollfd;
#else
#include <arpa/inet.h>
#include <netinet/in.h>
#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

UDPManager::UDPManager(NetworkManager &ref, PrometheusServer &metrics) : _NetworkManagerRef(ref), _metrics(metrics)
{
    _udpFd = socket(AF_INET, SOCK_DGRAM, 0);
    if (_udpFd == -1)
    {
#ifdef _WIN32
        int error = WSAGetLastError();
        throw std::runtime_error("UDP socket failed with error: " + std::to_string(error));
#else
        throw std::runtime_error("UDP socket failed");
#endif
    }

    _addr.sin_family = AF_INET;
    _addr.sin_addr.s_addr = INADDR_ANY;
    _addr.sin_port = htons(SERVER_PORT);

    int opt = 1;
#ifdef _WIN32
    if (setsockopt(_udpFd, SOL_SOCKET, SO_REUSEADDR, (const char *)&opt, sizeof(opt)) < 0)
#else
    if (setsockopt(_udpFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
#endif
        throw std::runtime_error("setsockopt(SO_REUSEADDR) failed");

    if (bind(_udpFd, (sockaddr *)&_addr, sizeof(_addr)) < 0)
    {
#ifdef _WIN32
        int error = WSAGetLastError();
        throw std::runtime_error("UDP bind failed with error: " + std::to_string(error));
#else
        throw std::runtime_error("UDP bind failed");
#endif
    }

    _pollFds.push_back({_udpFd, POLLIN, 0});
    std::cout << "[UDP] Socket initialized on port " << SERVER_PORT << std::endl;
}

UDPManager::~UDPManager()
{
    close(_udpFd);
}

void UDPManager::update()
{
#ifdef _WIN32
    int ret = WSAPoll(_pollFds.data(), _pollFds.size(), 0);
    if (ret == SOCKET_ERROR)
    {
        int error = WSAGetLastError();
        throw std::runtime_error("UDP poll failed with error: " + std::to_string(error));
    }
#else
    int ret = poll(_pollFds.data(), _pollFds.size(), 0);
    if (ret < 0)
        throw std::runtime_error("UDP poll failed");
#endif

    for (auto &pfd : _pollFds)
    {
        if (pfd.revents & POLLIN)
        {
            sockaddr_in client{};
            socklen_t len = sizeof(client);
            auto [opcode, payload] = receiveFrameUDP(_udpFd, client, len);

            if (opcode == OPCODE_UDP_AUTH)
            {
                _metrics.IncrementUDPReceived();
                _metrics.AddUDPBytes(payload.size() + 2); // opcode + length
                int code = deserializeInt(payload);
                std::cout << "[UDP] Received OPCODE_UDP_AUTH: " << code << " from: " << client.sin_addr.s_addr << "\n";
                Client *c = _NetworkManagerRef.getClientManager().getClientByCodeUDP(code);
                if (c != nullptr)
                {
                    c->setAdress(std::to_string(client.sin_addr.s_addr));
                    c->setTrueAddr(&client);
                }
                else
                {
                    std::cout << "[UDP] Client not found with UDP code " << code << "\n";
                }
            }
            else
            {
                Client *c =
                    _NetworkManagerRef.getClientManager().getClientByAdress(std::to_string(client.sin_addr.s_addr));
                if (c != nullptr)
                {
                    _NetworkManagerRef.getGameMediator().notify(static_cast<GameMediatorEvent>(opcode), payload);
                    _metrics.IncrementUDPReceived();
                    _metrics.AddUDPBytes(payload.size() + 2); // opcode + length
                }
                else
                    std::cout << "[UDP] Received from unknown client: " << payload
                              << " from: " << client.sin_addr.s_addr << "\n";
            }
        }
    }
}

void UDPManager::sendTo(const std::vector<sockaddr_in> &addrs, int opcode, const std::string &data)
{
    for (const sockaddr_in &addr : addrs)
    {
        sendFrameUDP(_udpFd, opcode, data, addr, sizeof(addr));
        _metrics.IncrementUDPSent();
        _metrics.AddUDPBytes(data.size() + 2); // opcode + length
    }
}
