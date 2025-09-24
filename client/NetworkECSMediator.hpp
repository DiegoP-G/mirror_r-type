#pragma once

#include "../transferData/opcode.hpp"
#include "Network/Receiver.hpp"
#include "Network/Sender.hpp"
#include <functional>
#include <string>
#include <unordered_map>

enum NetworkECSMediatorEvent
{
    SEND_DATA_TCP,
    SEND_DATA_UDP,
    UPDATE_DATA
};

class Sender;
class Receiver;

class NetworkECSMediator
{
  private:
    Sender *_sender = nullptr;
    Receiver *_receiver = nullptr;

    std::unordered_map<int, std::function<void(const std::string &, int)>> _mediatorMap;

  public:
    NetworkECSMediator();

    void notify(NetworkECSMediatorEvent event, const std::string &data, int opcode = -1);

    void setSender(Sender &sender)
    {
        _sender = &sender;
    }
    void setReceiver(Receiver &receiver)
    {
        _receiver = &receiver;
    }
};
