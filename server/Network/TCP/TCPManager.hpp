#pragma once

#include <arpa/inet.h>
#include <netinet/in.h>
#include <poll.h>
#include <string>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>

class NetworkManager;

class TCPManager
{
  private:
    int _listenFd;
    std::vector<pollfd> _pollFds;
    NetworkManager &_networkManagerRef;

  public:
    TCPManager(NetworkManager &ref);
    ~TCPManager();
    void update();
    void sendToClient(int fd, const std::string &msg);
};
