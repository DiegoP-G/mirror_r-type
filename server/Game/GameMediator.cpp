/*
** EPITECH PROJECT, 2025
** mirror_r-type
** File description:
** GameMediator
*/

#include "GameMediator.hpp"

GameMediator::GameMediator()
{
    _mediatorMap = {{GameMediatorEvent::TickLogic, [this]() -> void {}},
                    {GameMediatorEvent::SetupNetwork, [this]() -> void {}},
                    {GameMediatorEvent::TickNetwork, [this]() -> void { _networkManager.updateAllPoll(); }}};
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
