#pragma once
#include "../NetworkECSMediator.hpp"
#include <cstdint>
#include <iostream>
#include <netinet/in.h>
#include <string>

class NetworkECSMediator;

class Sender
{
  private:
    int _tcpSocket = -1;
    int _udpSocket = -1;
    sockaddr_in _serverAddr;

    NetworkECSMediator &_med;

  public:
    Sender(NetworkECSMediator &med) : _med(med), _tcpSocket(-1), _udpSocket(-1)
    {
        std::cout << "INIT SENDER" << std::endl;
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

    void sendTcp(uint8_t opcode, const std::string &payload);
    void sendUdp(uint8_t opcode, const std::string &payload);
};
