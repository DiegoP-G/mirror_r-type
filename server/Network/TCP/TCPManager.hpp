#pragma once
#include <arpa/inet.h>
#include <iostream>
#include <netinet/in.h>
#include <poll.h>
#include <string>
#include <sys/socket.h>
#include <unistd.h>
#include <unordered_map>
#include <vector>

class TCPManager {
private:
  int _listenFd;
  std::vector<pollfd> _pollFds;
  std::unordered_map<int, std::string> _clients;

public:
  TCPManager(int port);
  ~TCPManager();
  void update();
  void sendToClient(int fd, const std::string &msg);
};
