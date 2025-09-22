/*
** EPITECH PROJECT, 2025
** mirror_r-type
** File description:
** GameMediator
*/

#include "NetworkMediator.hpp"
#include "../Include/Errors/EventUnknown.hpp"

void NetworkMediator::notify(void *sender, const int &event,
                             const std::string &data) {
  switch (event) {
  case NetworkMediatorEvent::TCP:
    std::cout << "[NetworkMediator] Handling "
              << NetworkMediator::toString(NetworkMediatorEvent::TCP) << "\n";
    break;
  case NetworkMediatorEvent::UDP:
    std::cout << "[NetworkMediator] Handling "
              << NetworkMediator::toString(NetworkMediatorEvent::UDP) << "\n";
    break;
  default:
    throw UnknownEventException(event);
  }
}
