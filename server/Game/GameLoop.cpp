#include "GameLoop.hpp"
#include "GameMediator.hpp"

Orchestrator::GameLoop::GameLoop()
{
}

Orchestrator::GameLoop::~GameLoop()
{
}

void Orchestrator::GameLoop::loop()
{
    std::cout << "in loop game" << std::endl;
    _gameMediator.notify(GameMediatorEvent::SetupNetwork);
    while (1)
    {
        auto frameTime = std::chrono::milliseconds(16); // ~60 FPS (1000ms/60 ≈ 16.67ms)
        auto startTime = std::chrono::high_resolution_clock::now();
        _gameMediator.notify(GameMediatorEvent::TickNetwork);
        _gameMediator.notify(GameMediatorEvent::TickLogic, std::to_string(std::chrono::duration<float>(frameTime).count()));

    }
}