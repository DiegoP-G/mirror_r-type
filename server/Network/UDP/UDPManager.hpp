#pragma once
#include <arpa/inet.h>
#include <netinet/in.h>
#include <poll.h>
#include <string>
#include <sys/socket.h>
#include <unistd.h>
#include <unordered_map>
#include <vector>

class NetworkManager;

class UDPManager
{
  private:
    int _udpFd;
    std::vector<pollfd> _pollFds;
    NetworkManager &_NetworkManagerRef;
    sockaddr_in _addr;

  public:
    UDPManager(NetworkManager &ref);
    ~UDPManager();
    void update();
    void sendTo(std::vector<int> sockets, int opcode, const std::string &msg);
    void handleReceival(uint8_t opcode, const std::string &data);
};
