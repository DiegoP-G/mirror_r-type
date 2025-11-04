#pragma once

#include "../../transferData/hashUtils.hpp"
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

    EVP_PKEY *_serverPubKey;
    // Key first, IV second
    std::unordered_map<int, std::pair<std::vector<uint8_t>, std::vector<uint8_t>>> _aesKeyIVMap;
    std::unordered_map<int, uint32_t> _udpSequenceNumbers;
    std::thread _tcpThread;
    std::thread _udpThread;
    std::atomic<bool> _shouldStop{false};

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

    // SENF DATA TO ECS

    void addNewPlayer(int socket);

    void sendAllEntitiesToClient(int clientFd);

    // SEND DATA FROM ECS

    void sendDataAllClientUDP(std::string data, int opcode);
    void sendDataAllClientTCP(std::string data, int opcode);
    void sendDataToLobbyTCP(std::shared_ptr<Lobby>, const std::string &data, int opcode);
    void sendDataToLobbyUDP(std::shared_ptr<Lobby>, const std::string &data, int opcode);
    void sendDataToLobbyUDPExcept(std::shared_ptr<Lobby> lobby, const std::string &data, int opcode,
                                  int excludeClientFd);

    void stopNetworkLoops();
    void startNetworkLoops();
    TCPManager &getTCPManager()
    {
        return _TCPManager;
    };

    void setServerPubKey(EVP_PKEY *key)
    {
        if (key)
        {
            _serverPubKey = key;
        }
    }

    void setAesKeyIV(int clientFd, const std::vector<uint8_t> &key, const std::vector<uint8_t> &iv)
    {
        _aesKeyIVMap[clientFd] = std::make_pair(key, iv);
    }

    std::optional<std::vector<uint8_t>> getAesKey(int clientFd)
    {
        if (_aesKeyIVMap.find(clientFd) != _aesKeyIVMap.end())
        {
            return _aesKeyIVMap[clientFd].first;
        }
        return std::nullopt;
    }

    std::optional<std::vector<uint8_t>> getAesIV(int clientFd)
    {
        if (_aesKeyIVMap.find(clientFd) != _aesKeyIVMap.end())
        {
            return _aesKeyIVMap[clientFd].second;
        }
        return std::nullopt;
    }

    EVP_PKEY *getServerPubKey()
    {
        return _serverPubKey;
    };
};
