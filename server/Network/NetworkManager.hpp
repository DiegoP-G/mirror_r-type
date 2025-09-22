#pragma once

#include "ClientManager.hpp"
#include "NetworkMediator.hpp"
#include <poll.h>

#define SERVER_PORT 8080

class NetworkManager
{
  private:
    ClientManager _clientManager;
    NetworkMediator _networkMediator;

  public:
    NetworkManager();
    ~NetworkManager();

    void updateAllPoll();
};
