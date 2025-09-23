#pragma once
#include <arpa/inet.h>
#include <netinet/in.h>
#include <poll.h>
#include <string>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>

class NetworkManager;

class UDPManager
{
  private:
    int _udpFd;
    std::vector<pollfd> _pollFds;
    NetworkManager &_NetworkManagerRef;

  public:
    UDPManager(NetworkManager &ref);
    ~UDPManager();
    void update();
    void sendTo(const std::string &ip, int port, const std::string &msg);
};
