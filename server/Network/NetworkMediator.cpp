/*
** EPITECH PROJECT, 2025
** mirror_r-type
** File description:
** GameMediator
*/

#include "NetworkMediator.hpp"
#include "../Include/Errors/EventUnknown.hpp"
#include "UDP/UDPManager.hpp"
#include <iostream>

NetworkMediator::NetworkMediator(NetworkManager &ref) : _netWorkManegerRef(ref), _UDPManager(ref), _TCPManager(ref)
{
}

void NetworkMediator::notify(const int &event, const std::string &data)
{
    switch (event)
    {
    case NetworkMediatorEvent::TCP:
        std::cout << "[NetworkMediator] Handling " << NetworkMediator::toString(NetworkMediatorEvent::TCP) << "\n";
        break;
    case NetworkMediatorEvent::UDP:
        std::cout << "[NetworkMediator] Handling " << NetworkMediator::toString(NetworkMediatorEvent::UDP) << "\n";
        break;
    default:
        throw UnknownEventException(event);
    }
}
