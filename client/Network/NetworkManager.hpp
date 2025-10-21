#pragma once

#include "../NetworkECSMediator.hpp"
#include "../RType.hpp"
#include "Receiver.hpp"
#include "Sender.hpp"
#include <atomic>
#include <netinet/in.h>
#include <poll.h>
#include <vector>
#include "../../transferData/hashUtils.hpp"

class NetworkManager
{
  private:
    NetworkECSMediator &_med;
    Sender &_sender;
    Receiver &_receiver;

    int _tcpSocket;
    int _udpSocket;
    std::vector<struct pollfd> _pollFds;

    sockaddr_in _serverAddr;
    std::atomic<bool> _shouldStop;

    EVP_PKEY *_clientPubKey;
    EVP_PKEY *_serverPubKey;
    std::string _aesKey;
    std::string _aesIV;

  public:
    NetworkManager(NetworkECSMediator &med, Sender &sender, Receiver &receiver);
    ~NetworkManager();

    bool setup(const char *serverIp = "127.0.0.1", int port = 8080);
    void loop();
    void stop();

    void handleReceival();
    void handleSend();
    Sender &getSender();
    Receiver &getReceiver();

    void setServerPubKey(EVP_PKEY *key)
    {
      if (key) {
        _serverPubKey = key;
      }
    }

    void setClientPubKey(EVP_PKEY *key)
    {
      if (key) {
        _clientPubKey = key;
      }
    }

    void setAesKey(std::string &key)
    {
      _aesKey = key;
    }

    void setAesIV(std::string &iv)
    {
      _aesIV = iv;
    }

    EVP_PKEY *getServerPubKey() { return _serverPubKey; };
    EVP_PKEY *getClientPubKey() { return _clientPubKey; };
    std::string &getAesKey() { return _aesKey; };
    std::string &getAesIV() { return _aesIV; };
};
