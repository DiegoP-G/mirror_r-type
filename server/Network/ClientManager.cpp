#include "ClientManager.hpp"
#include "../../transferData/opcode.hpp"
#include "../AdministratorPanel.hpp"
#include "Client.hpp"
#include <iostream>

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

void ClientManager::addClient(const Client &c)
{
    std::cout << "=== addClient DEBUG START ===" << std::endl;

    int sock = c.getSocket();
    std::cout << "[DEBUG] Client name: " << c.getName() << std::endl;
    std::cout << "[DEBUG] Client socket: " << sock << std::endl;

    // Print current map keys
    std::cout << "[DEBUG] Existing clients in map:";
    for (const auto &pair : _clients)
    {
        std::cout << " " << pair.first;
    }
    std::cout << std::endl;

    // Check for duplicate
    if (_clients.find(sock) != _clients.end())
    {
        std::cerr << "[DEBUG] Client with socket " << sock << " already exists. Skipping insertion.\n";
        std::cout << "=== addClient DEBUG END ===" << std::endl;
        return;
    }

    if (_adminPanel)
    {
        std::string logMessage = "Client connected: " + c.getName() + " socket: " + std::to_string(sock);
        _adminPanel->addLog(logMessage);
    }
    auto result = _clients.emplace(sock, c);
    if (result.second)
        std::cout << "[DEBUG] Client successfully added." << std::endl;
    else
        std::cerr << "[DEBUG] Failed to add client!" << std::endl;

    std::cout << "[DEBUG] Clients after insertion:";
    for (const auto &pair : _clients)
    {
        std::cout << " " << pair.first;
    }
    std::cout << std::endl;

    std::cout << "=== addClient DEBUG END ===" << std::endl;
}

bool ClientManager::removeClient(int socket)
{
    auto it = _clients.find(socket);
    if (it != _clients.end())
    {
        std::cout << "Removing client: " << it->second.getName() << " (socket " << socket << ")\n";
        _clients.erase(it);
#ifdef _WIN32
        closesocket(socket);
#else
        close(socket);
#endif // WIN_32
        return true;
    }
    else
    {
        std::cerr << "Client with socket " << socket << " not found.\n";
        return false;
    }
}

void ClientManager::broadcast(const std::string &message)
{
    for (auto &[sock, client] : _clients)
    {
        client.sendMessage(message);
    }
}

Client *ClientManager::getClient(int socket)
{
    auto it = _clients.find(socket);
    if (it != _clients.end())
    {
        return &(it->second);
    }
    return nullptr;
}

Client *ClientManager::getClientByCodeUDP(int code)
{
    for (auto &c : _clients)
    {
        if (c.second.getCodeUDP() == code)
            return &(c.second);
    }
    return nullptr;
}
Client *ClientManager::getClientByAdress(std::string adress)
{
    for (auto &c : _clients)
    {
        if (c.second.getAdress() == adress)
            return &(c.second);
    }
    return nullptr;
}

std::unordered_map<int, Client> &ClientManager::getClientsMap()
{
    return _clients;
}

bool ClientManager::isBannedIP(std::string ip)
{
    if (_adminPanel)
    {
        return _adminPanel->isBannedIp(ip);
    }
    return false;
}

void ClientManager::addAdminPanelLog(std::string log)
{
    if (_adminPanel)
    {
        _adminPanel->addLog(log);
    }
}

bool ClientManager::checkLoginCreds(const std::string &username, const std::string &password)
{
    printf("Check login creds\n");
    if (_adminPanel)
    {
        printf("In _admin panel\n");
        return _adminPanel->getSqlApi().validateCredentials(username, password);
    }
    printf("Out _admin panel\n");
    return false;
}

bool ClientManager::addNewPlayerEntry(const std::string &username, const std::string &password)
{
    if (_adminPanel)
    {
        try
        {
            _adminPanel->getSqlApi().addPlayerEntry(username, password);
        }
        catch (std::exception &e)
        {
            throw;
        }
        return true;
    }
    return false;
}