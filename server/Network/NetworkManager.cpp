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

void NetworkManager::updateEntities(std::string data)
{
    int opcode = OPCODE_WORLD_UPDATE;
    auto map = _clientManager.getClientsMap();

    std::vector<int> socketsClients;
    for (auto c : map)
    {
        socketsClients.push_back(c.first);
    }
    _UDPManager.sendTo(socketsClients, opcode, data);
}
