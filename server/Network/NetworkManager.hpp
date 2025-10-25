#pragma once

#include "../../transferData/opcode.hpp"
#include "../../transferData/hashUtils.hpp"
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
    std::vector<uint8_t> _aesKey;
    std::vector<uint8_t> _aesIV;

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
