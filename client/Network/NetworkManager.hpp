#pragma once

#include "Receiver.hpp"
#include "Sender.hpp"
#include <netinet/in.h>
#include <poll.h>
#include <vector>

class NetworkManager
{
  private:
    Sender _sender;
    Receiver _receiver;

    int _tcpSocket;
    int _udpSocket;
    std::vector<struct pollfd> _pollFds;

    sockaddr_in _serverAddr;

  public:
    NetworkManager();
    ~NetworkManager();

    bool setup(const char *serverIp = "127.0.0.1", int port = 8080);
    void loop();

    void handleReceival();
    void handleSend();
    Sender &getSender();
    Receiver &getReceiver();
};
