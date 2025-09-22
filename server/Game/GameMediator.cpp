/*
** EPITECH PROJECT, 2025
** mirror_r-type
** File description:
** GameMediator
*/

#include "GameMediator.hpp"

GameMediator::GameMediator()
{
    _networkManager = NetworkManager();
    _mediatorMap = {{GameMediatorEvent::TickLogic, [this]() -> void { }},
                    {GameMediatorEvent::SetupNetwork,
                     [this]() -> void {
                         _networkManager.setupSockets(SERVER_PORT);
                         _networkManager.setupPolls();
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
