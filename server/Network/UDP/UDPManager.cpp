#include "UDPManager.hpp"
#include "../../../transferData/opcode.hpp"
#include "../../../transferData/transferData.hpp"
#include "../../Game/GameMediator.hpp"
#include "../NetworkManager.hpp"
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <string>

UDPManager::UDPManager(NetworkManager &ref) : _NetworkManagerRef(ref)
{
    _udpFd = socket(AF_INET, SOCK_DGRAM, 0);
    if (_udpFd < 0)
        throw std::runtime_error("UDP socket failed");

    _addr.sin_family = AF_INET;
    _addr.sin_addr.s_addr = INADDR_ANY;
    _addr.sin_port = htons(SERVER_PORT);

    int opt = 1;
    if (setsockopt(_udpFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
        throw std::runtime_error("setsockopt(SO_REUSEADDR) failed");
    if (bind(_udpFd, (sockaddr *)&_addr, sizeof(_addr)) < 0)
        throw std::runtime_error("UDP bind failed");

    _pollFds.push_back({_udpFd, POLLIN, 0});
}

UDPManager::~UDPManager()
{
    close(_udpFd);
}

void UDPManager::update()
{
    int ret = poll(_pollFds.data(), _pollFds.size(), 0);
    if (ret < 0)
        throw std::runtime_error("UDP poll failed");

    for (auto &pfd : _pollFds)
    {
        if (pfd.revents & POLLIN)
        {
            sockaddr_in client{};
            socklen_t len = sizeof(client);
            auto [opcode, payload] = receiveFrameUDP(_udpFd, client, len);

            if (opcode == OPCODE_UDP_AUTH)
            {
                std::cout << "[UDP] Received OPCODE_UDP_AUTH : " << deserializeInt(payload)
                          << " from: " << client.sin_addr.s_addr << "\n";
                Client *c = _NetworkManagerRef.getClientManager().getClientByCodeUDP(deserializeInt(payload));
                if (c != nullptr)
                {
                    c->setAdress(std::to_string(client.sin_addr.s_addr));
                }
                else
                {
                    std::cout << "[UDP] Client not found with UDP code " << std::to_string(deserializeInt(payload))
                              << "\n";
                }
            }
            else
            {
                Client *c =
                    _NetworkManagerRef.getClientManager().getClientByAdress((std::to_string(client.sin_addr.s_addr)));
                // if (c != nullptr)
                //     _NetworkManagerRef.getGameMediator().notify(static_cast<GameMediatorEvent>(opcode), payload);
                // else
                // std::cout << "[UDP] Received from Client Not found: " << (payload)
                //           << " from: " << client.sin_addr.s_addr << "\n";
                if (opcode == OPCODE_PLAYER_INPUT)
                    std::cout << "[UDP] Received player input " << std::endl;
            }
        }
    }
}

void UDPManager::sendTo(std::vector<int> sockets, int opcode, const std::string &data)
{
    for (int socket : sockets)
    {
        sendFrameUDP(socket, opcode, data, _addr, sizeof(_addr));
    }
}