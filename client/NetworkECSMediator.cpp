#include "NetworkECSMediator.hpp"
#include "Network/Receiver.hpp"
#include <cstdint>
#include <exception>
#include <iostream>
#include <stdexcept>

NetworkECSMediator::NetworkECSMediator()
{
    _mediatorMap = {
        {static_cast<int>(NetworkECSMediatorEvent::SEND_DATA_TCP),
         [this](const std::string &data, uint8_t opcode) {
             std::cout << "SENDING TCP" << std::endl;
             _sender->sendTcp(opcode, data);
         }},
        {static_cast<int>(NetworkECSMediatorEvent::SEND_DATA_UDP),
         [this](const std::string &data, uint8_t opcode) {
             std::cout << "SENDING UDP" << std::endl;
             std::cout << data << " " << (int)opcode << std::endl;
             _sender->sendUdp(opcode, data);
             //  std::cout << "FINISH UDP" << std::endl;
         }},
        {static_cast<int>(NetworkECSMediatorEvent::UPDATE_DATA),
         [this](const std::string &data, uint8_t opcode) { std::cout << "RECEIVED DATA: " << data << std::endl; }}};
}

void NetworkECSMediator::notify(NetworkECSMediatorEvent event, const std::string &data, uint8_t opcode)
{
    auto it = _mediatorMap.end();
    try
    {
        it = _mediatorMap.find((int)event);
    }
    catch (std::exception &e)
    {
        std::cerr << e.what() << std::endl;
    }

    if (it != _mediatorMap.end())
    {
        it->second(data, opcode);
    }
    else
    {
        throw std::runtime_error("notify: No handler registered for event");
    }
}
