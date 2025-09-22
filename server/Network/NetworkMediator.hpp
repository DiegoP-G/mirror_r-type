#pragma once

#include "../Mediator/IMediator.hpp"
#include "TCP/TCPManager.hpp"
#include "UDP/UDPManager.hpp"
#include <string>

enum NetworkMediatorEvent
{
    UDP,
    TCP
};

class NetworkMediator : public IMediator
{
    std::string toString(NetworkMediatorEvent event)
    {
        switch (event)
        {
        case UDP:
            return "UdpMessage";
        case TCP:
            return "TcpMessage";
        default:
            return "UnknownEvent";
        }
    }

  private:
    TCPManager _TCPManager;
    UDPManager _UDPManager;
    NetworkManager &_netWorkManegerRef;

  public:
    NetworkMediator(NetworkManager &ref);
    ~NetworkMediator() = default;

    void notify(const int &event, const std::string &data = "") override;
    inline void sendMessageToAll(const std::string &data) override
    {
        return;
    };
};
