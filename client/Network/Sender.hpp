#pragma once
#include <netinet/in.h>
#include <string>

class NetworkManager;

class Sender
{
  private:
    NetworkManager &_networkManagerRef;
    int _tcpSocket{-1};
    int _udpSocket{-1};
    sockaddr_in _serverAddr{};

  public:
    Sender(NetworkManager &ref) : _networkManagerRef(ref) {};

    void setTcpSocket(int socket)
    {
        _tcpSocket = socket;
    }
    void setUdpSocket(int socket)
    {
        _udpSocket = socket;
    }
    void setServerAddr(const sockaddr_in &addr)
    {
        _serverAddr = addr;
    }

    void sendTcp(uint8_t opcode, const std::string &payload);
    void sendUdp(uint8_t opcode, const std::string &payload);
};