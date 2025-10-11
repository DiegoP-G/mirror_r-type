#include "GameLoop.hpp"
#include "GameMediator.hpp"
#include <chrono>

Orchestrator::GameLoop::GameLoop()
{
}

Orchestrator::GameLoop::~GameLoop()
{
}

void Orchestrator::GameLoop::loop()
{
    _gameMediator.notify(GameMediatorEvent::SetupNetwork);

    const double tickRate = 60.0; // 60 TPS
    const std::chrono::duration<double> tickDuration(1.0 / tickRate);
    auto previousTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> accumulatedTime(0);
    _gameMediator.notify(GameMediatorEvent::InitECS, "");

    while (true)
    {

        _gameMediator.notify(GameMediatorEvent::TickNetwork);

        auto currentTime = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> frameTime = currentTime - previousTime;
        previousTime = currentTime;

        accumulatedTime += frameTime;

        while (accumulatedTime >= tickDuration)
        {
            double deltaTime = tickDuration.count(); // 1/60 s
            _gameMediator.notify(GameMediatorEvent::TickLogic, std::to_string(deltaTime));

            accumulatedTime -= tickDuration;
        }
    }
}
