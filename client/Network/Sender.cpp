#include "Sender.hpp"
#include "../../transferData/transferData.hpp"
#include "../NetworkECSMediator.hpp"
#include <cstdint>
#include <iostream>
#include <unistd.h>

void Sender::sendTcp(uint8_t opcode, const std::string &payload)
{
    if (_tcpSocket == -1)
    {
        std::cerr << "[Sender] TCP socket not set!" << std::endl;
        return;
    }

    try
    {
        sendFrameTCP(_tcpSocket, opcode, payload);
        std::cout << "[Sender] Sent TCP frame (opcode=" << std::to_string(opcode) << ", size=" << payload.size() << ")"
                  << std::endl;
    }
    catch (const std::exception &e)
    {
        std::cerr << "[Sender] Error sending TCP frame: " << e.what() << std::endl;
    }
}

void Sender::sendUdp(uint8_t opcode, const std::string &payload)
{
    std::cout << "HERE" << std::endl;
    // std::cout << (int)opcode << "|" << payload << "|" << std::endl;
    std::cout << _udpSocket << std::endl;
    // if (_udpSocket == -1)
    // {
    //     std::cerr << "[Sender] UDP socket not set!" << std::endl;
    //     return;
    // }

    // std::cout << "x" << std::endl;
    // try
    // {
    // std::cout << "x" << std::endl;
    // std::cout << payload << std::endl;
    //     sendFrameUDP(_udpSocket, opcode, payload, _serverAddr, sizeof(_serverAddr));
    // }
    // catch (const std::exception &e)
    // {
    //     std::cerr << "[Sender] Error sending UDP frame: " << e.what() << std::endl;
    // }
}
