#pragma once
#include <functional>
#include <iostream>
#ifdef _WIN32
    #ifndef NOMINMAX
        #define NOMINMAX
    #endif

    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
    #include <windows.h>
#else
    #include <arpa/inet.h>
    #include <netinet/in.h>
    #include <sys/socket.h>
    #include <unistd.h>
#endif
//#include <netinet/in.h>
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
    }

    Receiver &operator=(const Receiver &other)
    {
        if (this != &other)
        {
            _tcpBuffer = other._tcpBuffer;
            _tcpSocket = other._tcpSocket;
            _udpSocket = other._udpSocket;
            _serverAddr = other._serverAddr;
            _handlers = other._handlers;
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
