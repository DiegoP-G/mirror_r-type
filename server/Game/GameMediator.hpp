#pragma once
#include "../Include/Errors/EventUnknown.hpp"
#include "../Mediator/IMediator.hpp"
#include <functional>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include "../Network/NetworkManager.hpp"

enum GameMediatorEvent { TickNetwork, TickLogic, SetupNetwork };

class GameMediator : public IMediator {
private:
    NetworkManager _networkManager;
  std::unordered_map<GameMediatorEvent, std::function<void()>> _mediatorMap;

public:
  GameMediator(NetworkManager& networkManager);
  ~GameMediator() = default;

  void notify(const int &event, const std::string &data = "") override;
  inline void sendMessageToAll(const std::string &data) override { return; };
  std::string toString(GameMediatorEvent event) {
    switch (event) {
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
