#pragma once
#include <functional>
#include <netinet/in.h>
#include <string>
#include <unordered_map>

class NetworkECSMediator;

class Receiver
{
  private:
    /* data */
    std::string _tcpBuffer;
    int _tcpSocket;
    int _udpSocket;
    sockaddr_in _serverAddr;
    NetworkECSMediator &_med;

    std::unordered_map<int, std::function<void(const std::string &, int opcode)>> _handlers;

  public:
    Receiver(NetworkECSMediator &med);

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
