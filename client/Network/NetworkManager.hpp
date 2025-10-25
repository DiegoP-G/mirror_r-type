#pragma once

#include <atomic>
#include <netinet/in.h>
#include <poll.h>
#include "../NetworkECSMediator.hpp"
#include "../RType.hpp"
#include "Receiver.hpp"
#include "Sender.hpp"
#include "transferData/hashUtils.hpp"

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

    EVP_PKEY *_serverPubKey;
    std::vector<uint8_t> _aesKey;
    std::vector<uint8_t> _aesIV;

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

    void setAesKey(std::vector<uint8_t> &key)
    {
      _aesKey = key;
    }

    void setAesIV(std::vector<uint8_t> &iv)
    {
      _aesIV = iv;
    }

    EVP_PKEY *getServerPubKey() { return _serverPubKey; };
    const std::vector<uint8_t> &getAesKey() { return _aesKey; };
    const std::vector<uint8_t> &getAesIV() { return _aesIV; };
};
