#include "GameLoop.hpp"
#include "GameMediator.hpp"
#include <memory>

Orchestrator::GameLoop::GameLoop()
{
    _gameMediator = GameMediator();
}

Orchestrator::GameLoop::~GameLoop()
{}

void Orchestrator::GameLoop::loop()
{
    std::cout << "in loop game" << std::endl;
    _gameMediator.notify(GameMediatorEvent::SetupNetwork);
    while (1)
    {
        _gameMediator.notify(GameMediatorEvent::TickNetwork);
        _gameMediator.notify(GameMediatorEvent::TickLogic);
    }
}