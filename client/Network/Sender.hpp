#pragma once
#include "../NetworkECSMediator.hpp"
#include <iostream>
#include <netinet/in.h>
#include <string>

class NetworkECSMediator;

class Sender
{
  private:
    int _tcpSocket;
    int _udpSocket;
    sockaddr_in _serverAddr;

    NetworkECSMediator &_med;

  public:
    Sender(NetworkECSMediator &med) : _med(med), _tcpSocket(-1), _udpSocket(-1)
    {
    }

    void setTcpSocket(int socket)
    {
        std::cout << "TCP " << socket << std::endl;
        _tcpSocket = socket;
    }

    void setUdpSocket(int socket)
    {
        std::cout << "UDP " << socket << std::endl;
        _udpSocket = socket;
    }

    void setServerAddr(const sockaddr_in &addr)
    {
        _serverAddr = addr;
    }

    void sendTcp(int opcode, const std::string &payload);
    void sendUdp(int opcode, const std::string &payload);
};
