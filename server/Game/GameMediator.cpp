/*
** EPITECH PROJECT, 2025
** mirror_r-type
** File description:
** GameMediator
*/

#include "GameMediator.hpp"

GameMediator::GameMediator()
{
    _mediatorMap = {{GameMediatorEvent::TickLogic, [this](const std::string& data) -> void { _rTypeServer.update(std::stof(data));}},
                    {GameMediatorEvent::SetupNetwork, [this](const std::string& data) -> void {}},
                    {GameMediatorEvent::TickNetwork, [this](const std::string& data) -> void { _networkManager.updateAllPoll(); }}};
}

void GameMediator::notify(const int &event, const std::string &data)
{
    const GameMediatorEvent &gameEvent = static_cast<GameMediatorEvent>(event);
    auto it = _mediatorMap.find(gameEvent);

    if (it != _mediatorMap.end())
    {
        it->second(data);
    }
    else
    {
        throw UnknownEventException(gameEvent);
    }
}
