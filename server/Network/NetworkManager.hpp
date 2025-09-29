#pragma once

#include "../../transferData/opcode.hpp"
#include "../Game/GameMediator.hpp"
#include "Client.hpp"
#include "ClientManager.hpp"
#include "TCP/TCPManager.hpp"
#include "UDP/UDPManager.hpp"
#include <poll.h>
#include <unistd.h>
#include <vector>

#define SERVER_PORT 8081

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

    void updateAllPoll();

    // SENF DATA TO ECS

    void addNewPlayer(int socket);

    // SEND DATA FROM ECS

    void updateEntities(std::string data);
};
