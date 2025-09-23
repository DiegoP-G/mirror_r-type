#pragma once

#include "Receiver.hpp"
#include "Sender.hpp"
#include <vector>
#include <poll.h>
#include <netinet/in.h>

class NetworkManager {
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

    bool setup(const char* serverIp = "127.0.0.1", int port = 8080);
    void loop();
    Sender& getSender();
    Receiver& getReceiver();
};
