#pragma once

#include "../../transferData/opcode.hpp"
#include "../Game/GameMediator.hpp"
#include "Client.hpp"
#include "ClientManager.hpp"
#include "TCP/TCPManager.hpp"
#include "UDP/UDPManager.hpp"
#include <cstdint>
#include <functional>
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
#include "../Prometheus/PrometheusServer.hpp"
#include <unordered_map>
#include <vector>

constexpr int SERVER_PORT = 8081;

class GameMediator;

class NetworkManager
{
  private:
    GameMediator &_gameMediator;
    PrometheusServer _metrics;
    ClientManager _clientManager;
    UDPManager _UDPManager;
    TCPManager _TCPManager;

  public:
    NetworkManager(GameMediator &ref);
    ~NetworkManager();

    inline ClientManager &getClientManager()
    {
        return _clientManager;
    }

    inline GameMediator &getGameMediator()
    {
        return _gameMediator;
    }

    void updateAllPoll();

    // SENF DATA TO ECS

    void addNewPlayer(int socket);

    void sendAllEntitiesToClient(int clientFd);

    // SEND DATA FROM ECS

    void sendDataAllClientUDP(std::string data, int opcode);
    void sendDataAllClientTCP(std::string data, int opcode);
    void sendDataToLobbyTCP(std::shared_ptr<Lobby>, const std::string &data, int opcode);
    void sendDataToLobbyUDP(std::shared_ptr<Lobby>, const std::string &data, int opcode);

    TCPManager &getTCPManager()
    {
        return _TCPManager;
    };
};
