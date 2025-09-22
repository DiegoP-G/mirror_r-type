#pragma once
#include "../Include/Errors/EventUnknown.hpp"
#include "../Mediator/IMediator.hpp"
#include <iostream>
#include <string>
#include <vector>

class GameLogic;
class CombatManager;
class ClientManager;
class NetworkMediator;

enum GameMediatorEvent { TickNetwork, TickLogic };

class GameMediator : public IMediator {
  std::string toString(GameMediatorEvent event) {
    switch (event) {
    case TickNetwork:
      return "Ticked Network";
    case TickLogic:
      return "Ticked Logic";
    default:
      return "UnknownEvent";
    }
  }

public:
  GameMediator() = default;
  ~GameMediator() = default;

  void notify(const int &event, const std::string &data = "") override;
  inline void sendMessageToAll(const std::string &data) override { return; };
};
