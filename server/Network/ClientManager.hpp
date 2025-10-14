#pragma once
#include "Client.hpp"
#include <string>
#include <unordered_map>

class AdministratorPanel;

class ClientManager
{
  private:
    std::unordered_map<int, Client> _clients;
    AdministratorPanel *_adminPanel;

  public:
    ClientManager() : _adminPanel(nullptr)
    {
    }

    void setAdministratorPanel(AdministratorPanel &adminPanel)
    {
        _adminPanel = &adminPanel;
    }

    void addClient(const Client &c);
    bool removeClient(int socket);
    void broadcast(const std::string &message);
    Client *getClient(int socket);
    Client *getClientByCodeUDP(int code);
    Client *getClientByAdress(std::string adress);

    std::unordered_map<int, Client> &getClientsMap();
};