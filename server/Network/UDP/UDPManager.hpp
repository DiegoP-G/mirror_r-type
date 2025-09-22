#pragma once
#include "../../transferData/transferData.hpp"
#include <arpa/inet.h>
#include <iostream>
#include <netinet/in.h>
#include <poll.h>
#include <string>
#include <sys/socket.h>
#include <unistd.h>
#include <unordered_map>
#include <vector>

class UDPManager
{
  private:
    int _udpFd;
    std::vector<pollfd> _pollFds;

  public:
    UDPManager(int port);
    ~UDPManager();
    void update();
    void sendTo(const std::string &ip, int port, const std::string &msg);
};
