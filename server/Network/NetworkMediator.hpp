#pragma once
#include <iostream>
#include <string>
#include <vector>
#include "../Mediator/IMediator.hpp"
#include "../Include/Errors/EventUnknown.hpp"


enum NetworkMediatorEvent {
    UDP,
    TCP
};

class NetworkMediator : public IMediator {
    std::string toString(NetworkMediatorEvent event) {
        switch (event) {
            case UDP:   return "UdpMessage";
            case TCP:   return "TcpMessage";
            default:            return "UnknownEvent";
        }
    }

public:
    NetworkMediator()=default;
    ~NetworkMediator()=default;

    void notify(void* sender, const int& event, const std::string& data) override;

};
