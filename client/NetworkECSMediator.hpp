#pragma once

#include "../transferData/opcode.hpp"
#include "Network/Receiver.hpp"
#include "Network/Sender.hpp"
#include <cstdint>
#include <functional>
#include <string>
#include <unordered_map>

enum NetworkECSMediatorEvent
{
    SEND_DATA_TCP,
    SEND_DATA_UDP,
    UPDATE_DATA,
    PLAYER_ID
};

class Sender;
class Receiver;
class RTypeGame;

class NetworkECSMediator
{
  private:
    Sender *_sender{nullptr};
    Receiver *_receiver{nullptr};
    RTypeGame *_game{nullptr};

    std::unordered_map<int, std::function<void(const std::string &, uint8_t)>> _mediatorMap;

  public:
    NetworkECSMediator();

    void setSender(Sender *s)
    {
        _sender = s;
    }
    void setReceiver(Receiver *r)
    {
        _receiver = r;
    }
    void setRTypeGame(RTypeGame *g)
    {
        _game = g;
    }

    void reset()
    {
        _sender = nullptr;
        _receiver = nullptr;
        _game = nullptr;
        _mediatorMap.clear();
    }

    void notify(NetworkECSMediatorEvent event, const std::string &data, uint8_t opcode = -1);
    void receiveEntitiesUpdates(const std::vector<uint8_t> &data);
    void deserializeHealth(const std::vector<uint8_t> &data);
    void deserializeMovements(const std::vector<uint8_t> &data);
};
