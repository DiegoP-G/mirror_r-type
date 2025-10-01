#pragma once

#include <arpa/inet.h>
#include <cstddef>
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
    struct sockaddr_in _addr;

  public:
    TCPManager(NetworkManager &ref);
    ~TCPManager();
    void update();
    void sendTo(std::vector<int> fd, int opcode, const std::string &msg);

    void acceptConnection();
    void handlePollin(size_t &i, pollfd &pfd);
};
