#pragma once

#include <arpa/inet.h>
#include <cstddef>
#include <map>
#include <netinet/in.h>
#include <poll.h>
#include <queue>
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
    std::map<int, std::queue<std::pair<uint8_t, std::string>>> _pendingMessages;

    struct sockaddr_in _addr;

  public:
    TCPManager(NetworkManager &ref);
    ~TCPManager();
    void update();
    void sendTo(std::vector<int> fd, int opcode, const std::string &msg);

    void acceptConnection();
    void handlePollin(size_t &i, pollfd &pfd);
    void handlePollout(size_t i, pollfd &pfd);

    void queueMessage(int fd, uint8_t opcode, const std::string &payload);
    void setPollOut(int fd, bool enable);
};
