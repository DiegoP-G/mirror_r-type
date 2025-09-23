#pragma once
#include "Client.hpp"
#include <string>
#include <unordered_map>

class ClientManager
{
  private:
    std::unordered_map<int, Client> _clients;

  public:
    ClientManager() = default;

    void addClient(const Client &c);
    void removeClient(int socket);
    void broadcast(const std::string &message);
    Client *getClient(int socket);
    Client *getClientByCodeUDP(int code);
    Client *getClientByAdress(std::string adress);

    std::unordered_map<int, Client> &getClientsMap();
};