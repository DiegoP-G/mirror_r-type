#pragma once
#include "../Include/Errors/EventUnknown.hpp"
#include "../Mediator/IMediator.hpp"
#include "../Network/NetworkManager.hpp"
#include "RTypeServer.hpp"
#include <chrono>
#include <functional>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

enum GameMediatorEvent
{
    TickNetwork,
    TickLogic,
    AddPlayer,
    SetupNetwork,
    UpdateEntities = 0x21,
    UpdatePlayers = 0x23,
    UpdateEnemies = 0x24,
    UpdateProjectiles = 0x25,
    PlayerInput = 0x26,
    EntityCreated = 0x27,
    EntityDestroyed = 0x28,
    MovementUpdate = 0x29,
    
};

class NetworkManager;
class RTypeServer;

class GameMediator : public IMediator
{
  private:
    NetworkManager &_networkManager;
    RTypeServer &_rTypeServer;

    std::unordered_map<GameMediatorEvent, std::function<void(const std::string &)>> _mediatorMap;

  public:
    GameMediator();
    ~GameMediator() = default;

    void notify(const int &event, const std::string &data = "") override;
    inline void sendMessageToAll(const std::string &data) override
    {
        return;
    };

    std::vector<std::string> getAllActiveEntities();


    std::string toString(GameMediatorEvent event)
    {
        switch (event)
        {
        case TickNetwork:
            return "Ticked Network";
        case TickLogic:
            return "Ticked Logic";
        case SetupNetwork:
            return "Setup Network";
        default:
            return "UnknownEvent";
        }
    }
};
