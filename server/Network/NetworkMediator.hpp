#pragma once
#include "../Include/Errors/EventUnknown.hpp"
#include "../Mediator/IMediator.hpp"
#include <iostream>
#include <string>
#include <vector>

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

  public:
    NetworkMediator() = default;
    ~NetworkMediator() = default;

    void notify(const int &event, const std::string &data = "") override;
    inline void sendMessageToAll(const std::string &data) override
    {
        return;
    };
};
