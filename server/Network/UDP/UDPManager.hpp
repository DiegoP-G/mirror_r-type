#pragma once

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
#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

#include "../../Prometheus/PrometheusServer.hpp"
#include <string>
#include <unordered_map>
#include <vector>

class NetworkManager;

class UDPManager
{
  private:
#ifdef _WIN32
    SOCKET _udpFd;
    std::vector<WSAPOLLFD> _pollFds;
#else
    int _udpFd;
    std::vector<struct pollfd> _pollFds;
#endif
    NetworkManager &_NetworkManagerRef;
    PrometheusServer &_metrics;
    sockaddr_in _addr;

  public:
    UDPManager(NetworkManager &ref, PrometheusServer &metrics);
    ~UDPManager();
    void update();
    void sendTo(const std::vector<sockaddr_in> &addrs, int opcode, const std::string &data);
    void handleReceival(uint8_t opcode, const std::string &data);
};
