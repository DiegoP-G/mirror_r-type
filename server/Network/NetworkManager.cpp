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

// Vérifier si l'adresse est valide (sin_family initialisé)
void NetworkManager::sendDataAllClientUDP(std::string data, int opcode)
{
    auto map = _clientManager.getClientsMap();

    std::vector<sockaddr_in> clientAddrs;
    for (auto &c : map)
    {
        // Ne pas envoyer si l'adresse UDP n'est pas encore configurée
        sockaddr_in addr = c.second.getTrueAddr();

        // Vérifier si l'adresse est valide (sin_family initialisé)
        if (addr.sin_family == AF_INET && addr.sin_port != 0)
        {
            clientAddrs.push_back(addr);
        }
        else
        {
            std::cout << "[UDP] Skipping client " << c.second.getSocket() << " (UDP not authenticated yet)"
                      << std::endl;
        }
    }

    if (!clientAddrs.empty())
    {
        _UDPManager.sendTo(clientAddrs, opcode, data);
    }
}

void NetworkManager::sendDataAllClientTCP(std::string data, int opcode)
{
    auto map = _clientManager.getClientsMap();

    std::vector<int> clientSocket;
    for (auto c : map)
    {
        // clientSocket.push_back(c.second.getSocket());
        std::cout << "send to client opcide == " << opcode << "\n";
        _TCPManager.sendMessage(c.second.getSocket(), opcode, data);
    }
}

void NetworkManager::sendAllEntitiesToClient(int clientFd)
{
    std::cout << "[NetworkManager] Sending all existing entities to new client " << clientFd << std::endl;

    // Récupérer toutes les entités actives depuis le serveur de jeu
    std::vector<std::string> allEntities = _gameMediator.getAllActiveEntities();

    std::cout << "[NetworkManager] Sending " << allEntities.size() << " entities to client " << clientFd << std::endl;

    // Envoyer chaque entité au nouveau client via TCP
    for (const auto &entityData : allEntities)
    {
        if (!entityData.empty())
        {
            _TCPManager.sendMessage(clientFd, OPCODE_ENTITY_CREATE, entityData);
        }
    }

    std::cout << "[NetworkManager] Finished sending entities to client " << clientFd << std::endl;
}