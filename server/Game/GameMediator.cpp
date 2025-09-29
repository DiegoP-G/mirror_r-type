/*
** EPITECH PROJECT, 2025
** mirror_r-type
** File description:
** GameMediator
*/

#include "GameMediator.hpp"
#include "../Network/NetworkManager.hpp"

GameMediator::GameMediator() : _networkManager(*new NetworkManager(*this)), _rTypeServer(*new RTypeServer(*this))
{
    _mediatorMap = {
        {GameMediatorEvent::TickLogic, [this](const std::string &data) -> void { _rTypeServer.run(std::stof(data)); }},
        {GameMediatorEvent::SetupNetwork, [this](const std::string &data) -> void {}},
        {GameMediatorEvent::TickNetwork, [this](const std::string &data) -> void { _networkManager.updateAllPoll(); }},
        {GameMediatorEvent::AddPlayer, [this](const std::string &data) -> void { _rTypeServer.createPlayer(data); }},
        {GameMediatorEvent::UpdateEntities,
         [this](const std::string &data) -> void { _networkManager.updateEntities(data); }},
        {GameMediatorEvent::PlayerInput,
         [this](const std::string &data) -> void { _rTypeServer.handlePlayerInput(data); }},
    };
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
