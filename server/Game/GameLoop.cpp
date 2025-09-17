#include "GameLoop.hpp"



Orchestrator::GameLoop::GameLoop()
{
    _GameMediator = std::make_shared<GameMediator>();
}

Orchestrator::GameLoop::~GameLoop()
{
}

void Orchestrator::GameLoop::loop()
{
    while (1) {
    }
}