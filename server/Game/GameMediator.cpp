/*
** EPITECH PROJECT, 2025
** mirror_r-type
** File description:
** GameMediator
*/

#include "GameMediator.hpp"

GameMediator::GameMediator(NetworkManager &networkManager) : _networkManager(networkManager)
{
    _mediatorMap = {{GameMediatorEvent::TickLogic, [this]() -> void { std::cout << "TickLogic: none" << std::endl; }},
                    {GameMediatorEvent::SetupNetwork,
                     [this]() -> void {
                         _networkManager.setupPolls();
                         _networkManager.setupSockets(SERVER_PORT);
                     }},
                    {GameMediatorEvent::TickNetwork, [this]() -> void { _networkManager.pollOnce(); }}};
}

void GameMediator::notify(const int &event, const std::string &data)
{
    const GameMediatorEvent &gameEvent = static_cast<GameMediatorEvent>(event);
    auto it = _mediatorMap.find(gameEvent);

    if (it != _mediatorMap.end())
    {
        it->second();
    }
    else
    {
        throw UnknownEventException(gameEvent);
    }
}
