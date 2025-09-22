#pragma once
#include <string>

class Client
{
  private:
    int _socketFd;
    std::string _name;
    bool _connected;
    std::string _buffer;

  public:
    Client() = default;
    Client(const std::string &clientName, int sockFd);
    ~Client();

    void sendMessage(const std::string &msg);
    int getSocket() const;
    bool isConnected() const;
    std::string getName() const;
    void setSocket(int clientSock);
    void setConnected(bool state);
    inline std::string &getBuffer()
    {
        return _buffer;
    };
};