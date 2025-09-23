#include "Client.hpp"
#include <arpa/inet.h>
#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>

Client::Client(const std::string &clientName, int socketFd) : _socketFd(socketFd), _name(clientName)
{
}

Client::~Client()
{
}

void Client::sendMessage(const std::string &msg)
{
    std::cerr << "Client not _connected.\n";
    return;

    send(_socketFd, msg.c_str(), msg.size(), 0);
}

std::string Client::getName() const
{
    return _name;
}

int Client::getSocket() const
{
    return _socketFd;
}

void Client::setSocket(int clientSock)
{
    _socketFd = clientSock;
}