#pragma once

#include "../NetworkECSMediator.hpp"
#include "../RType.hpp"
#include "Receiver.hpp"
#include "Sender.hpp"
#include <atomic>
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
using SocketType = SOCKET;
#include <windows.h>

#else
using SocketType = int;
#include <netinet/in.h>
#include <poll.h>
#endif
#include <vector>

class NetworkManager
{
  private:
    NetworkECSMediator &_med;
    Sender &_sender;
    Receiver &_receiver;

    SocketType _tcpSocket;
    SocketType _udpSocket;

#ifdef _WIN32
    std::vector<WSAPOLLFD> _pollFds;
#else
    std::vector<struct pollfd> _pollFds;
#endif

    sockaddr_in _serverAddr;
    std::atomic<bool> _shouldStop;

  public:
    NetworkManager(NetworkECSMediator &med, Sender &sender, Receiver &receiver);
    ~NetworkManager();

    bool setup(const char *serverIp = "127.0.0.1", int port = 8080);
    void loop();
    void stop();

    void handleReceival();
    void handleSend();
    Sender &getSender();
    Receiver &getReceiver();
};
