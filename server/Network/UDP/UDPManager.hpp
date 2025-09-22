#pragma once

class UDPManager {
private:
  int _udpFd;
  std::vector<pollfd> _pollFds;

public:
  UDPManager(int port);
  ~UDPManager();
  void update();
  void sendTo(const std::string &ip, int port, const std::string &msg);
};
