#pragma once

#include <arpa/inet.h>
#include <map>
#include <netinet/in.h>
#include <poll.h>
#include <queue>
#include <string>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>
#include "../../transferData/opcode.hpp"

class NetworkManager;

class TCPManager
{
  private:
    int _listenFd;
    std::vector<pollfd> _pollFds;
    NetworkManager &_networkManagerRef;

    // Buffer d'écriture par client (données brutes à envoyer)
    std::map<int, std::string> _writeBuffers;

    struct sockaddr_in _addr;

    void handleNewConnection();
    void handleClientRead(int fd, size_t &index);
    void handleClientWrite(int fd);
    void sendAESKey(int clientFd);

  public:
    TCPManager(NetworkManager &ref);
    ~TCPManager();

    void update();
    void sendMessage(int fd, uint8_t opcode, const std::string &payload);
};