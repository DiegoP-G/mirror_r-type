#include "Network/Receiver.hpp"
#include "Network/Sender.hpp"
#include <string>

#pragma once

enum NetworkECSMediatorEvent
{
    SendData,
    UpdateData
};

class NetworkECSMediator
{
  private:
    // ECS
    Sender _sender;
    Receiver _receiver;

  public:
    void notify(const NetworkECSMediator &event, const std::string &data);
};