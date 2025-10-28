#include "NetworkManager.hpp"
#include "../../transferData/opcode.hpp"
#include "../../transferData/transferData.hpp"
#include <cstdint>
#include <fcntl.h>
#include <memory>
#include <string>
#include <sys/types.h>

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

NetworkManager::NetworkManager(GameMediator &ref)
    : _gameMediator(ref), _UDPManager(*this), _TCPManager(*this), _serverPubKey(nullptr)
{
    EVP_PKEY *serverKey = generateRSAKeyPair(2048);
    if (!serverKey)
    {
        std::cerr << "Failed to generate server DH key pair" << std::endl;
        return;
    }
    setServerPubKey(serverKey);
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
    // std::cout << "NEW SOCKETTT" << socket << std::endl;
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
        _TCPManager.sendMessage(c.second.getSocket(), opcode, data);
    }
}

void NetworkManager::sendDataToLobbyTCP(std::shared_ptr<Lobby> lobby, const std::string &data, int opcode)
{
    auto players = lobby->getPlayers();

    for (int fd : players)
    {
        std::cout << "[TCP] Sending to lobby player fd: " << fd << std::endl;
        _TCPManager.sendMessage(fd, opcode, data);
    }
}

void NetworkManager::sendAllEntitiesToClient(int clientFd)
{
    std::string allEntities = _gameMediator.getAllActiveEntitiesFromLobby(clientFd);

    if (!allEntities.empty())
    {
        _TCPManager.sendMessage(clientFd, OPCODE_ENTITY_CREATE, allEntities);
    }
}

void NetworkManager::sendDataToLobbyUDP(std::shared_ptr<Lobby> lobby, const std::string &data, int opcode)
{
    auto players = lobby->getPlayers();
    std::vector<sockaddr_in> validAddrs;

    for (int fd : players)
    {
        auto clientOpt = _clientManager.getClient(fd);
        if (!clientOpt)
            continue;

        sockaddr_in addr = clientOpt->getTrueAddr();
        if (addr.sin_family == AF_INET && addr.sin_port != 0)
            validAddrs.push_back(addr);
        else
            std::cout << "[UDP] Skipping client " << fd << " (UDP not authenticated yet)\n";
    }

    if (!validAddrs.empty())
        _UDPManager.sendTo(validAddrs, opcode, data);
}

void NetworkManager::sendDataToLobbyUDPExcept(std::shared_ptr<Lobby> lobby, const std::string &data, int opcode,
                                              int excludeClientFd)
{
    auto players = lobby->getPlayers();
    std::vector<sockaddr_in> validAddrs;

    for (int fd : players)
    {
        // ✅ Sauter l'émetteur
        if (fd == excludeClientFd)
        {
            std::cout << "[UDP] Skipping sender client fd: " << fd << std::endl;
            continue;
        }

        auto clientOpt = _clientManager.getClient(fd);
        if (!clientOpt)
            continue;

        sockaddr_in addr = clientOpt->getTrueAddr();
        if (addr.sin_family == AF_INET && addr.sin_port != 0)
            validAddrs.push_back(addr);
        else
            std::cout << "[UDP] Skipping client " << fd << " (UDP not authenticated yet)\n";
    }

    if (!validAddrs.empty())
    {
        std::cout << "[UDP] Broadcasting to " << validAddrs.size() << " clients (excluding " << excludeClientFd << ")"
                  << std::endl;
        _UDPManager.sendTo(validAddrs, opcode, data);
    }
}