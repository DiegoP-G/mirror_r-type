#pragma once

#include "ClientManager.hpp"
#include "TCP/TCPManager.hpp"
#include "UDP/UDPManager.hpp"
#include <poll.h>

#define SERVER_PORT 8080

class NetworkManager
{
  private:
    ClientManager _clientManager;
    UDPManager _UDPManager;
    TCPManager _TCPManager;

  public:
    NetworkManager();
    ~NetworkManager();

    void updateAllPoll();
};
