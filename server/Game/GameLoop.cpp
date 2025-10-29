#include "GameLoop.hpp"
#include "AdministratorPanel.hpp"
#include "GameMediator.hpp"
#include <chrono>
#include <thread>

Orchestrator::GameLoop::GameLoop()
{
    std::cout << "GameLoop initialized." << std::endl;
}

Orchestrator::GameLoop::~GameLoop()
{
}

// GameLoop.cpp (modified loop)
void Orchestrator::GameLoop::loop()
{
    _gameMediator.notify(GameMediatorEvent::SetupNetwork);

    AdministratorPanel adminPanel(_gameMediator.getNetworkManager());
    adminPanel.setClientManager(_gameMediator.getNetworkManager().getClientManager());
    adminPanel.setLobbyManager(_gameMediator.getLobbyManager());
    _gameMediator.getNetworkManager().getClientManager().setAdministratorPanel(adminPanel);

    // Start the admin panel in a separate thread
    std::thread adminPanelThread([&adminPanel]() { adminPanel.run(); });

    // Start blocking network loops
    _gameMediator.getNetworkManager().startNetworkLoops();

    const double tickRate = 60.0; // 60 TPS
    const std::chrono::duration<double> tickDuration(1.0 / tickRate);
    auto previousTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> accumulatedTime(0);

    while (true)
    {
        auto currentTime = std::chrono::high_resolution_clock::now();
        accumulatedTime += currentTime - previousTime;
        previousTime = currentTime;

        while (accumulatedTime >= tickDuration)
        {
            double deltaTime = tickDuration.count(); // 1/60 s
            // _gameMediator.notify(GameMediatorEvent::TickLogic, std::to_string(deltaTime));
            accumulatedTime -= tickDuration;
        }

        // Optional: sleep a tiny amount to avoid 100% CPU if tick processing is very fast
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    // Stop network loops on exit
    _gameMediator.getNetworkManager().stopNetworkLoops();

    adminPanelThread.join();
}
