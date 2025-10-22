#include "Client.hpp"
#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#include <windows.h>
#else
#include <arpa/inet.h>
#include <netinet/in.h>
#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

#include <cstring>
#include <iostream>
#include <sys/types.h>

Client::Client(const std::string &clientName, int socketFd) : _socketFd(socketFd), _name(clientName)
{
    std::memset(&_trueAddr, 0, sizeof(_trueAddr));
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