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
    std::cout << "in loop game" << std::endl;
    while (1)
    {
    }
}