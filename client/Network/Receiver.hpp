#pragma once
#include <functional>
#include <netinet/in.h>
#include <string>
#include <unordered_map>

class NetworkManager;

class Receiver
{
  private:
    /* data */
    NetworkManager &_networkManagerRef;
    int _tcpSocket;
    int _udpSocket;
    sockaddr_in _serverAddr;

    std::unordered_map<int, std::function<void(const std::string &)>> _handlers;

  public:
    Receiver(NetworkManager &ref);

    void receiveTCPMessage();
    void receiveUDPMessage();

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

    void onCodeUdp(const std::string &payload);
    void onCloseConnection(const std::string &payload);
    void onChatBroadcast(const std::string &payload);
};
