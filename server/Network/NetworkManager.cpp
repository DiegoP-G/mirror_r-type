#include "NetworkManager.hpp"
#include "../../transferData/opcode.hpp"
#include "../../transferData/transferData.hpp"
#include <arpa/inet.h>
#include <cstdint>
#include <fcntl.h>
#include <netinet/in.h>
#include <poll.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

NetworkManager::NetworkManager(GameMediator &ref) : _gameMediator(ref), _UDPManager(*this), _TCPManager(*this)
{
}

NetworkManager::~NetworkManager()
{
}

void NetworkManager::updateAllPoll()
{
    _UDPManager.update();
    _TCPManager.update();
}

void NetworkManager::addNewPlayer(int socket)
{
    _gameMediator.notify(AddPlayer, serializeInt(socket));
}

void NetworkManager::sendDataAllClientUDP(std::string data, int opcode)
{
    auto map = _clientManager.getClientsMap();

    std::vector<sockaddr_in> clientAddrs;
    for (auto c : map)
    {
        clientAddrs.push_back(c.second.getTrueAddr());
    }
    _UDPManager.sendTo(clientAddrs, opcode, data);
}

void NetworkManager::sendDataAllClientTCP(std::string data, int opcode)
{
    auto map = _clientManager.getClientsMap();

    std::vector<int> clientSocket;
    for (auto c : map)
    {
        clientSocket.push_back(c.second.getSocket());
    }
    _TCPManager.sendTo(clientSocket, opcode, data);
}
