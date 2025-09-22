#include "GameLoop.hpp"
#include "GameMediator.hpp"

Orchestrator::GameLoop::GameLoop() {
  _GameMediator = std::make_shared<GameMediator>();
}

Orchestrator::GameLoop::~GameLoop() {}

void Orchestrator::GameLoop::loop() {
  std::cout << "in loop game" << std::endl;
  _GameMediator->notify(GameMediatorEvent::SetupNetwork);
  while (1) {
    _GameMediator->notify(GameMediatorEvent::TickNetwork);
    _GameMediator->notify(GameMediatorEvent::TickLogic);
  }
}