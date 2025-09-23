#pragma once

#include "ClientManager.hpp"
#include "TCP/TCPManager.hpp"
#include "UDP/UDPManager.hpp"
#include <poll.h>

#define SERVER_PORT 8081

class NetworkManager
{
  private:
    ClientManager _clientManager;
    UDPManager _UDPManager;
    TCPManager _TCPManager;

  public:
    NetworkManager();
    ~NetworkManager();
    inline ClientManager &getClientManager()
    {
        return _clientManager;
    }

    void updateAllPoll();
};
