/*
** EPITECH PROJECT, 2025
** mirror_r-type
** File description:
** GameMediator
*/

#include "GameMediator.hpp"

void GameMediator::notify(const int &event, const std::string &data) {
  switch (event) {
  case GameMediatorEvent::TickLogic:
    std::cout << "[GameMediator] Handling "
              << GameMediator::toString(GameMediatorEvent::TickLogic) << "\n";
    break;
  case GameMediatorEvent::TickNetwork:
    std::cout << "[GameMediator] Handling "
              << GameMediator::toString(GameMediatorEvent::TickNetwork) << "\n";
    break;
  default:
    throw UnknownEventException(event);
  }
}
