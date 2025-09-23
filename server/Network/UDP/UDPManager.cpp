#include "UDPManager.hpp"
#include "../../../transferData/opcode.hpp"
#include "../../../transferData/transferData.hpp"
#include "../NetworkManager.hpp"
#include <cstddef>
#include <stdexcept>
#include <string>

UDPManager::UDPManager(NetworkManager &ref) : _NetworkManagerRef(ref)
{
    _udpFd = socket(AF_INET, SOCK_DGRAM, 0);
    if (_udpFd < 0)
        throw std::runtime_error("UDP socket failed");

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(SERVER_PORT);

    if (bind(_udpFd, (sockaddr *)&addr, sizeof(addr)) < 0)
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
            }
            else
            {
                Client *c =
                    _NetworkManagerRef.getClientManager().getClientByAdress((std::to_string(client.sin_addr.s_addr)));
                if (c != nullptr)
                    std::cout << "[UDP] Received : " << (payload) << " from: " << c->getName() << "\n";
                else
                    std::cout << "[UDP] Received : " << (payload) << " from: " << client.sin_addr.s_addr << "\n";
            }
        }
    }
}

void UDPManager::sendTo(const std::string &ip, int port, const std::string &msg)
{
    sockaddr_in dest{};
    dest.sin_family = AF_INET;
    dest.sin_port = htons(port);
    inet_pton(AF_INET, ip.c_str(), &dest.sin_addr);

    sendto(_udpFd, msg.data(), msg.size(), 0, (sockaddr *)&dest, sizeof(dest));
}