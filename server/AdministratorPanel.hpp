#pragma once
#include "../client/assetsPath.hpp"
#include "Network/ClientManager.hpp"
#include "Lobby/LobbyManager.hpp"
#include "NetworkManager.hpp"
#include "sqlAPI.hpp"
#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <SFML/Window.hpp>
#include <iostream>
#include <memory>
#include <unordered_map>

constexpr int WINDOWWIDTH = 800;
constexpr int WINDOWHEIGTH = 600;

class ClientManager;
class LobbyManager;

class AdministratorPanel
{
  private:
    std::unique_ptr<sf::Font> _font;
    ClientManager *_clientManager;
    LobbyManager *_lobbyManager;
    NetworkManager &_networkManager;
    sqlAPI _sqlApi;

    std::unordered_map<int, sf::RectangleShape> _kickButtons;
    std::unordered_map<int, sf::RectangleShape> _banButtons;

    float _playerListScrollOffset;
    float _logsScrollOffset;

    std::vector<std::string> _logs;

    void handleEvents(sf::RenderWindow &window);
    std::vector<std::tuple<sf::Text, sf::RectangleShape, sf::RectangleShape>> buildPlayerList(
        sf::RenderWindow &window, const sf::FloatRect &playerListArea);
    void drawPlayerList(sf::RenderWindow &window, sf::FloatRect playerListArea);
    void drawLogs(sf::RenderWindow &window, sf::FloatRect logsArea);
    void scrollLogic(sf::RenderWindow &window, sf::Event &event);
    void kickPlayer(sf::RenderWindow &window, sf::Event &event);
    void banPlayer(sf::RenderWindow &window, sf::Event &event);

  public:
    AdministratorPanel(NetworkManager &networkManager);

    void setClientManager(ClientManager &clientManager);
    void setLobbyManager(LobbyManager &clientManager);

    void addLog(std::string log)
    {
        if (_clientManager)
            _logs.push_back(log);
    };
    void run();

    bool isBannedIp(std::string);
    sqlAPI &getSqlApi()
    {
        return _sqlApi;
    };
};