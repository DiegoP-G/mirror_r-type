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
};
