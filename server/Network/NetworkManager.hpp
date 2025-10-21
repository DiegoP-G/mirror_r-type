#pragma once

#include "../../transferData/opcode.hpp"
#include "../Game/GameMediator.hpp"
#include "Client.hpp"
#include "ClientManager.hpp"
#include "TCP/TCPManager.hpp"
#include "UDP/UDPManager.hpp"
#include <cstdint>
#include <functional>
#include <poll.h>
#include <unistd.h>
#include <unordered_map>
#include <vector>

constexpr int SERVER_PORT = 8081;

class GameMediator;

class NetworkManager
{
  private:
    GameMediator &_gameMediator;
    ClientManager _clientManager;
    UDPManager _UDPManager;
    TCPManager _TCPManager;

    EVP_PKEY *_serverPubKey;
    EVP_PKEY *_clientPubKey;
    std::string _aesKey;
    std::string _aesIV;

  public:
    NetworkManager(GameMediator &ref);
    ~NetworkManager();

    inline ClientManager &getClientManager()
    {
        return _clientManager;
    }

    inline GameMediator &getGameMediator()
    {
        return _gameMediator;
    }

    void updateAllPoll();

    // SENF DATA TO ECS

    void addNewPlayer(int socket);

    void sendAllEntitiesToClient(int clientFd);

    // SEND DATA FROM ECS

    void sendDataAllClientUDP(std::string data, int opcode);
    void sendDataAllClientTCP(std::string data, int opcode);
    void sendDataToLobbyTCP(std::shared_ptr<Lobby>, const std::string &data, int opcode);
    void sendDataToLobbyUDP(std::shared_ptr<Lobby>, const std::string &data, int opcode);

    TCPManager &getTCPManager()
    {
        return _TCPManager;
    };

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
