#include "Sender.hpp"
#include "../../transferData/transferData.hpp"
#include "../NetworkECSMediator.hpp"
#include <iostream>
#include <unistd.h>

Sender::Sender(NetworkECSMediator &med) : _med(med)
{
}

void Sender::sendTcp(int opcode, const std::string &payload)
{
    std::cout << opcode << "|" << payload << std::endl;
    std::cout << "here1" << std::endl;
    if (_tcpSocket == -1)
    {
        std::cerr << "[Sender] TCP socket not set!" << std::endl;
        return;
    }

    std::cout << "here1" << std::endl;
    try
    {
        sendFrameTCP(_tcpSocket, opcode, payload);
        std::cout << "here1" << std::endl;
        std::cout << "[Sender] Sent TCP frame (opcode=" << std::to_string(opcode) << ", size=" << payload.size() << ")"
                  << std::endl;
        std::cout << "here1" << std::endl;
    }
    catch (const std::exception &e)
    {
        std::cerr << "[Sender] Error sending TCP frame: " << e.what() << std::endl;
    }
}

void Sender::sendUdp(int opcode, const std::string &payload)
{
    if (_udpSocket == -1)
    {
        std::cerr << "[Sender] UDP socket not set!" << std::endl;
        return;
    }

    try
    {
        sendFrameUDP(_udpSocket, opcode, payload, _serverAddr, sizeof(_serverAddr));
        std::cout << "[Sender] Sent UDP frame (opcode=" << std::to_string(opcode) << ", size=" << payload.size() << ")"
                  << std::endl;
    }
    catch (const std::exception &e)
    {
        std::cerr << "[Sender] Error sending UDP frame: " << e.what() << std::endl;
    }
}
