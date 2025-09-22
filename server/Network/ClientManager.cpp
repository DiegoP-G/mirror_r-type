#include "ClientManager.hpp"
#include <iostream>

void ClientManager::addClient(const Client &c)
{
    std::cout << "=== addClient DEBUG START ===" << std::endl;

    if (!c.isConnected())
    {
        std::cerr << "[DEBUG] Cannot add client: not connected.\n";
        std::cout << "=== addClient DEBUG END ===" << std::endl;
        return;
    }

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

    // Insert client
    auto result = _clients.emplace(sock, c);
    if (result.second)
        std::cout << "[DEBUG] Client successfully added." << std::endl;
    else
        std::cerr << "[DEBUG] Failed to add client!" << std::endl;

    // Print map keys after insertion
    std::cout << "[DEBUG] Clients after insertion:";
    for (const auto &pair : _clients)
    {
        std::cout << " " << pair.first;
    }
    std::cout << std::endl;

    std::cout << "=== addClient DEBUG END ===" << std::endl;
}

void ClientManager::removeClient(int socket)
{
    auto it = _clients.find(socket);
    if (it != _clients.end())
    {
        std::cout << "Removing client: " << it->second.getName() << " (socket " << socket << ")\n";
        _clients.erase(it);
    }
    else
    {
        std::cerr << "Client with socket " << socket << " not found.\n";
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

std::unordered_map<int, Client> &ClientManager::getClientsMap()
{
    return _clients;
}