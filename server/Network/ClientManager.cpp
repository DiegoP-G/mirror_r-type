#include "ClientManager.hpp"
#include <iostream>

void ClientManager::addClient(const Client &c) {
  if (!c.isConnected()) {
    std::cerr << "Cannot add client: not connected.\n";
    return;
  }
  _clients[c.getSocket()] = c;
  std::cout << "Added client: " << c.getName() << " (socket " << c.getSocket()
            << ")\n";
}

void ClientManager::removeClient(int socket) {
  auto it = _clients.find(socket);
  if (it != _clients.end()) {
    std::cout << "Removing client: " << it->second.getName() << " (socket "
              << socket << ")\n";
    _clients.erase(it);
  } else {
    std::cerr << "Client with socket " << socket << " not found.\n";
  }
}

void ClientManager::broadcast(const std::string &message) {
  for (auto &[sock, client] : _clients) {
    client.sendMessage(message);
  }
}

Client *ClientManager::getClient(int socket) {
  auto it = _clients.find(socket);
  if (it != _clients.end()) {
    return &(it->second);
  }
  return nullptr;
}

std::unordered_map<int, Client> &ClientManager::getClientsMap() {
  return _clients;
}