#pragma once
#include <functional>
#include <iostream>
#include <netinet/in.h>
#include <string>
#include <unordered_map>

class NetworkECSMediator;

class Receiver
{
  private:
    std::string _tcpBuffer;
    int _tcpSocket;
    int _udpSocket;
    sockaddr_in _serverAddr;
    NetworkECSMediator &_med;

    std::unordered_map<int, std::function<void(const std::string &, int opcode)>> _handlers;

  public:
    Receiver(NetworkECSMediator &med);

    Receiver(const Receiver &other)
        : _med(other._med), _tcpBuffer(other._tcpBuffer), _tcpSocket(other._tcpSocket), _udpSocket(other._udpSocket),
          _serverAddr(other._serverAddr), _handlers(other._handlers)
    {
        std::cout << "Receiver copied" << std::endl;
    }

    // Copy assignment operator
    Receiver &operator=(const Receiver &other)
    {
        if (this != &other)
        {
            _tcpBuffer = other._tcpBuffer;
            _tcpSocket = other._tcpSocket;
            _udpSocket = other._udpSocket;
            _serverAddr = other._serverAddr;
            _handlers = other._handlers;
            // _med cannot be reassigned, still references original
        }
        return *this;
    }

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
