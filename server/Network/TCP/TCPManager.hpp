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
#include <map>
#include <queue>
#include <string>
#include <vector>

class NetworkManager;

class TCPManager
{
  private:
#ifdef _WIN32
    SOCKET _listenFd;
    std::vector<WSAPOLLFD> _pollFds;
#else
    int _listenFd;
    std::vector<struct pollfd> _pollFds;
#endif
    NetworkManager &_networkManagerRef;
    PrometheusServer &_metrics;

    // Buffer d'écriture par client (données brutes à envoyer)
    std::map<int, std::string> _writeBuffers;

    struct sockaddr_in _addr;

    void handleNewConnection();
    void handleClientRead(int fd, size_t &index);
    void handleClientWrite(int fd);

  public:
    TCPManager(NetworkManager &ref, PrometheusServer &metrics);
    ~TCPManager();

    void update();
    void sendMessage(int fd, uint8_t opcode, const std::string &payload);
};