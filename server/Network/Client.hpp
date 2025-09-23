#pragma once
#include <string>

class Client
{
  private:
    int _socketFd;
    std::string _name;
    std::string _buffer;
    std::string _adress;
    int _code_UDP;

  public:
    Client() = default;
    Client(const std::string &clientName, int sockFd);
    ~Client();

    inline int getCodeUDP()
    {
        return _code_UDP;
    };
    inline void setCodeUDP(int c)
    {
        _code_UDP = c;
    };
    inline std::string getAdress()
    {
        return _adress;
    };
    inline void setAdress(std::string s)
    {
        _adress = s;
    };

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