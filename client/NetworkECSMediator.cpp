#include "NetworkECSMediator.hpp"
#include <exception>
#include <iostream>
#include <stdexcept>

NetworkECSMediator::NetworkECSMediator()
{
    _mediatorMap = {
        {static_cast<int>(NetworkECSMediatorEvent::SEND_DATA_TCP),
         [this](const std::string &data, int opcode) {
             std::cout << "sending tcp" << std::endl;
             _sender->sendTcp(opcode, data);
         }},
        {static_cast<int>(NetworkECSMediatorEvent::SEND_DATA_UDP),
         [this](const std::string &data, int opcode) {
             std::cout << "sending udp" << std::endl;
             _sender->sendUdp(opcode, data);
         }},
        {static_cast<int>(NetworkECSMediatorEvent::UPDATE_DATA), [this](const std::string &data, int opcode) {
             std::cout << "RECEIVED DATA: " << data << std::endl;
             // Call ECS functions here
         }}};
}

void NetworkECSMediator::notify(NetworkECSMediatorEvent event, const std::string &data, int opcode)
{
    std::cout << "x" << std::endl;
    std::cout << event << std::endl;
    std::cout << "data" << std::endl;
    std::cout << data << std::endl;
    std::cout << "opcode" << std::endl;
    std::cout << opcode << std::endl;
    auto it = _mediatorMap.end();
    try
    {
        it = _mediatorMap.find((int)event);
    }
    catch (std::exception &e)
    {
        std::cerr << e.what() << std::endl;
    }
    std::cout << "FOUND" << std::endl;

    if (it != _mediatorMap.end())
    {
        std::cout << "sgokqg" << std::endl;
        it->second(data, opcode);
    }
    else
    {
        throw std::runtime_error("notify: No handler registered for event");
    }
}
