#include "UDPManager.hpp"

UDPManager::UDPManager(int port) {
  _udpFd = socket(AF_INET, SOCK_DGRAM, 0);
  if (_udpFd < 0)
    throw std::runtime_error("UDP socket failed");

  sockaddr_in addr{};
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = INADDR_ANY;
  addr.sin_port = htons(port);

  if (bind(_udpFd, (sockaddr *)&addr, sizeof(addr)) < 0)
    throw std::runtime_error("UDP bind failed");

  _pollFds.push_back({_udpFd, POLLIN, 0});
}

UDPManager::~UDPManager() { close(_udpFd); }

void UDPManager::update() {
  int ret = poll(_pollFds.data(), _pollFds.size(), 0);
  if (ret < 0)
    throw std::runtime_error("UDP poll failed");

  for (auto &pfd : _pollFds) {
    if (pfd.revents & POLLIN) {
      char buf[1024];
      sockaddr_in client{};
      socklen_t len = sizeof(client);
      receiveFrameUDP(_udpFd, client, len);
      ssize_t n =
          recvfrom(_udpFd, buf, sizeof(buf), 0, (sockaddr *)&client, &len);
      if (n > 0) {
        std::string msg(buf, n);
        std::cout << "[UDP] From " << inet_ntoa(client.sin_addr) << ":"
                  << ntohs(client.sin_port) << " -> " << msg << "\n";
      }
    }
  }
}

void UDPManager::sendTo(const std::string &ip, int port,
                        const std::string &msg) {
  sockaddr_in dest{};
  dest.sin_family = AF_INET;
  dest.sin_port = htons(port);
  inet_pton(AF_INET, ip.c_str(), &dest.sin_addr);

  sendto(_udpFd, msg.data(), msg.size(), 0, (sockaddr *)&dest, sizeof(dest));
}