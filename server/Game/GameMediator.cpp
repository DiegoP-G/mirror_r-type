/*
** EPITECH PROJECT, 2025
** mirror_r-type
** File description:
** GameMediator
*/

#include "GameMediator.hpp"

void GameMediator::notify(void* sender, const int& event, const std::string& data)
{
    switch (event)
    {
        case GameMediatorEvent::PlayerDamaged:
            std::cout << "[GameMediator] Handling " << GameMediator::toString(GameMediatorEvent::PlayerDamaged) << "\n";
            break;
        case GameMediatorEvent::PlayerJoined:
            std::cout << "[GameMediator] Handling " << GameMediator::toString(GameMediatorEvent::PlayerJoined) << "\n";
            break;
        case GameMediatorEvent::ChatMessage:
            std::cout << "[GameMediator] Handling "  << GameMediator::toString(GameMediatorEvent::ChatMessage) << "\n";
            break;
        default:
            throw UnknownEventException(event);
    }
}

