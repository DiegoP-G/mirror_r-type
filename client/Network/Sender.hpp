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
    // Constructor
    Sender(NetworkECSMediator &med) : _med(med), _tcpSocket(-1), _udpSocket(-1)
    {
    }

    // Copy constructor
    Sender(const Sender &other)
        : _med(other._med), _tcpSocket(other._tcpSocket), _udpSocket(other._udpSocket), _serverAddr(other._serverAddr)
    {
        std::cout << "Sender copied" << std::endl;
    }

    // Copy assignment operator
    Sender &operator=(const Sender &other)
    {
        if (this != &other)
        {
            _tcpSocket = other._tcpSocket;
            _udpSocket = other._udpSocket;
            _serverAddr = other._serverAddr;
            // _med cannot be reassigned, still uses original reference
        }
        return *this;
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
