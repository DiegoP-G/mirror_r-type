#include "AdministratorPanel.hpp"
#include "Network/ClientManager.hpp"
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
    #include <sys/socket.h>
    #include <unistd.h>
    #include <poll.h>
#endif


AdministratorPanel::AdministratorPanel(NetworkManager &networkManager)
    : _clientManager(nullptr), _lobbyManager(nullptr), _playerListScrollOffset(0), _logsScrollOffset(0), _networkManager(networkManager),
      _sqlApi("db.sqlite")
{
    sf::Font font;
    if (!font.loadFromFile(PathFormater::formatAssetPath(fontPath)))
    {
        std::cerr << "Failed to load font: " << fontPath << std::endl;
        _font = nullptr;
        return;
    }
    _font = std::make_unique<sf::Font>(font);

    _logs.push_back("Started administrator panel.");
}

void AdministratorPanel::setClientManager(ClientManager &clientManager)
{
    _clientManager = &clientManager;
}

void AdministratorPanel::setLobbyManager(LobbyManager &lobby)
{
    _lobbyManager = &lobby;
}

void AdministratorPanel::scrollLogic(sf::RenderWindow &window, sf::Event &event)
{
    sf::Vector2i mousePos = sf::Mouse::getPosition(window);

    constexpr float LOG_HEIGHT = 20.0f;    // Height of each log entry
    constexpr float PLAYER_HEIGHT = 40.0f; // Height of each player entry

    // Check if the mouse is in the player list area
    if (mousePos.y < WINDOWHEIGTH * 0.7f)
    {
        // Calculate total height of the player list
        float totalPlayerHeight = _clientManager->getClientsMap().size() * PLAYER_HEIGHT;

        // Prevent scrolling if the total height is less than the visible area
        if (totalPlayerHeight > WINDOWHEIGTH * 0.7f)
        {
            _playerListScrollOffset += (-1 * event.mouseWheelScroll.delta) * 20; // Adjust scroll speed
            _playerListScrollOffset = std::max(_playerListScrollOffset, 0.0f);   // Prevent scrolling above the top
            _playerListScrollOffset = std::min(
                _playerListScrollOffset, totalPlayerHeight - WINDOWHEIGTH * 0.7f); // Prevent scrolling below the bottom
        }
    }
    else
    {
        // Calculate total height of the logs
        float totalLogHeight = _logs.size() * LOG_HEIGHT;

        // Prevent scrolling if the total height is less than the visible area
        if (totalLogHeight > WINDOWHEIGTH * 0.3f)
        {
            _logsScrollOffset += (-1 * event.mouseWheelScroll.delta) * 20; // Adjust scroll speed
            _logsScrollOffset = std::max(_logsScrollOffset, 0.0f);         // Prevent scrolling above the top
            _logsScrollOffset =
                std::min(_logsScrollOffset, totalLogHeight - WINDOWHEIGTH * 0.3f); // Prevent scrolling below the bottom
        }
    }
}

void AdministratorPanel::kickPlayer(sf::RenderWindow &window, sf::Event &event)
{
    sf::Vector2f mousePos(event.mouseButton.x, event.mouseButton.y);

    for (auto &[id, button] : _kickButtons)
    {
        if (button.getGlobalBounds().contains(mousePos))
        {
            if (_clientManager)
            {
                _logs.push_back("Kicking client " + std::to_string(id));
                _networkManager.getTCPManager().sendMessage(id, OPCODE_KICK_NOTIFICATION, "");

                auto lobby = _lobbyManager->getLobbyOfPlayer(id);
                if (lobby) {
                    lobby->removePlayer(id);
                }

                if (_clientManager->removeClient(id))
                {
                    _logs.push_back("Client " + std::to_string(id) + " was kicked.");
                }
                else
                {
                    _logs.push_back("Failed to kick client " + std::to_string(id) + ".");
                }
            }
            else
            {
                std::cerr << "AdministratorPanel: _clientManager not initialized when trying to kick client."
                          << std::endl;
            }
        }
    }
}

// Add a new method to handle banning a player
void AdministratorPanel::banPlayer(sf::RenderWindow &window, sf::Event &event)
{
    sf::Vector2f mousePos(event.mouseButton.x, event.mouseButton.y);

    for (auto &[id, button] : _banButtons)
    {
        if (button.getGlobalBounds().contains(mousePos))
        {
            if (_clientManager)
            {
                _logs.push_back("Banning client " + std::to_string(id));
                _networkManager.getTCPManager().sendMessage(id, OPCODE_BAN_NOTIFICATION, "");
                auto client = _clientManager->getClient(id);
                if (client)
                {
                    std::string ip = inet_ntoa(client->getTrueAddr().sin_addr);
                    _sqlApi.addBannedIp(ip);

                    auto lobby = _lobbyManager->getLobbyOfPlayer(id);
                    if (lobby) {
                        lobby->removePlayer(id);
                    }

                    _logs.push_back("Client " + std::to_string(id) + " with IP " + ip + " was banned.");
                }
                else
                {
                    _logs.push_back("Failed to ban client " + std::to_string(id) + ".");
                }
            }
            else
            {
                std::cerr << "AdministratorPanel: _clientManager not initialized when trying to ban client."
                          << std::endl;
            }
        }
    }
}

void AdministratorPanel::handleEvents(sf::RenderWindow &window)
{
    sf::Event event;
    while (window.pollEvent(event))
    {
        if (event.type == sf::Event::Closed)
        {
            window.close();
        }

        // Handle mouse wheel scrolling
        if (event.type == sf::Event::MouseWheelScrolled)
        {
            scrollLogic(window, event);
        }

        // Handle mouse clicks for the "Kick" buttons
        if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)
        {
            kickPlayer(window, event);
            banPlayer(window, event); // Handle "Ban" button clicks
        }
    }
}

std::vector<std::tuple<sf::Text, sf::RectangleShape, sf::RectangleShape>> AdministratorPanel::buildPlayerList(
    sf::RenderWindow &window, const sf::FloatRect &playerListArea)
{
    std::vector<std::tuple<sf::Text, sf::RectangleShape, sf::RectangleShape>> playerList;
    sf::Vector2i mousePos = sf::Mouse::getPosition(window);
    float y = playerListArea.top + 40 - _playerListScrollOffset; // Apply scroll offset

    for (auto &[id, client] : _clientManager->getClientsMap())
    {
        // Skip drawing if the text is outside the visible area
        if (y + 30 < playerListArea.top || y > playerListArea.top + playerListArea.height)
        {
            y += 40; // Move to the next line
            continue;
        }

        // Create player text
        sf::Text playerText;
        playerText.setFont(*_font.get());
        playerText.setString("ID: " + std::to_string(client.getSocket()) +
                             "  -  ADDRESS: " + inet_ntoa(client.getTrueAddr().sin_addr));
        playerText.setCharacterSize(24);
        playerText.setFillColor(sf::Color::White);
        playerText.setPosition(playerListArea.left + 10.f, y);

        // Create "Kick" button
        sf::RectangleShape kickButton(sf::Vector2f(100.f, 30.f));
        kickButton.setPosition(playerListArea.left + playerListArea.width - 240.f, y); // Adjust position
        kickButton.setFillColor(sf::Color::Green);

        // Check if the mouse is over the "Kick" button
        if (kickButton.getGlobalBounds().contains(static_cast<sf::Vector2f>(mousePos)))
        {
            kickButton.setFillColor(sf::Color::White);
        }

        // Store the "Kick" button for event handling
        _kickButtons[id] = kickButton;

        // Create "Ban" button
        sf::RectangleShape banButton(sf::Vector2f(100.f, 30.f));
        banButton.setPosition(playerListArea.left + playerListArea.width - 120.f, y); // Adjust position
        banButton.setFillColor(sf::Color::Red); // Default blue color for "Ban" button

        // Check if the mouse is over the "Ban" button
        if (banButton.getGlobalBounds().contains(static_cast<sf::Vector2f>(mousePos)))
        {
            banButton.setFillColor(sf::Color::White); // Change to white on hover
        }

        // Store the "Ban" button for event handling
        _banButtons[id] = banButton;

        // Add the player text, "Kick" button, and "Ban" button to the list
        playerList.emplace_back(playerText, kickButton, banButton);

        y += 40; // Move to the next line
    }
    return playerList;
}

void AdministratorPanel::drawPlayerList(sf::RenderWindow &window, sf::FloatRect playerListArea)
{
    // Draw the player list area
    sf::RectangleShape playerListBackground(sf::Vector2f(playerListArea.width, playerListArea.height));
    playerListBackground.setPosition(playerListArea.left, playerListArea.top);
    playerListBackground.setFillColor(sf::Color(50, 50, 50)); // Dark gray background
    window.draw(playerListBackground);

    // Define the height of the title
    constexpr float TITLE_HEIGHT = 40.0f;

    // Draw players
    float playerY = playerListArea.top + TITLE_HEIGHT - _playerListScrollOffset; // Start below the title
    std::vector<std::tuple<sf::Text, sf::RectangleShape, sf::RectangleShape>> playerList = buildPlayerList(window, playerListArea);

    for (auto &[player, button, banButton] : playerList)
    {
        // Skip drawing if the player is outside the visible area
        if (playerY + 40 < playerListArea.top + TITLE_HEIGHT || playerY > playerListArea.top + playerListArea.height)
        {
            playerY += 40; // Move to the next line
            continue;
        }

        // Draw the player text and "Kick" button
        player.setPosition(playerListArea.left + 10.f, playerY);
        button.setPosition(playerListArea.left + playerListArea.width - 240.f, playerY);

        window.draw(player);
        window.draw(button);

        // Draw "Kick" button label
        sf::Text kickButtonText;
        kickButtonText.setFont(*_font.get());
        kickButtonText.setString("Kick");
        kickButtonText.setCharacterSize(18);
        kickButtonText.setFillColor(sf::Color::Black); // Text color (black for contrast on white highlight)
        kickButtonText.setPosition(button.getPosition().x + 10.f, button.getPosition().y + 5.f);
        window.draw(kickButtonText);

        // Draw "Ban" button
        banButton.setPosition(playerListArea.left + playerListArea.width - 120.f, playerY);
        window.draw(banButton);

        // Draw "Ban" button label
        sf::Text banButtonText;
        banButtonText.setFont(*_font.get());
        banButtonText.setString("Ban");
        banButtonText.setCharacterSize(18);
        banButtonText.setFillColor(sf::Color::Black); // Text color (black for contrast on white highlight)
        banButtonText.setPosition(banButton.getPosition().x + 10.f, banButton.getPosition().y + 5.f);
        window.draw(banButtonText);

        playerY += 40; // Move to the next line
    }
}

void AdministratorPanel::drawLogs(sf::RenderWindow &window, sf::FloatRect logsArea)
{
    // Draw the logs area
    sf::RectangleShape logsBackground(sf::Vector2f(logsArea.width, logsArea.height));
    logsBackground.setPosition(logsArea.left, logsArea.top);
    logsBackground.setFillColor(sf::Color(30, 30, 30)); // Darker gray background
    window.draw(logsBackground);

    // Define the height of the title
    constexpr float TITLE_HEIGHT = 60.0f;

    // Draw logs
    float logY = logsArea.top - _logsScrollOffset; // Apply scroll offset
    for (const auto &log : _logs)
    {
        // Skip drawing if the log is outside the visible area
        if (logY + 20 < logsArea.top + TITLE_HEIGHT || logY > logsArea.top + logsArea.height)
        {
            logY += 20; // Move to the next line
            continue;
        }

        sf::Text logText;
        logText.setFont(*_font.get());
        logText.setString(log);
        logText.setCharacterSize(18);
        logText.setFillColor(sf::Color::White);
        logText.setPosition(logsArea.left + 10.f, logY);
        window.draw(logText);
        logY += 20; // Move to the next line
    }
}

void AdministratorPanel::run()
{
    sf::RenderWindow window(sf::VideoMode(WINDOWWIDTH, WINDOWHEIGTH), "Administrator Panel");

    // Define title height
    constexpr float TITLE_HEIGHT = 0.0f;

    // Define areas for player list and logs, subtracting the title height
    sf::FloatRect playerListArea(0, TITLE_HEIGHT, WINDOWWIDTH,
                                 WINDOWHEIGTH * 0.7f - TITLE_HEIGHT); // Top 70% for player list
    sf::FloatRect logsArea(0, WINDOWHEIGTH * 0.7f + TITLE_HEIGHT, WINDOWWIDTH,
                           WINDOWHEIGTH * 0.3f - TITLE_HEIGHT); // Bottom 30% for logs

    sf::Text playerText;
    playerText.setFont(*_font.get());
    playerText.setString("Player list:");
    playerText.setCharacterSize(24);
    playerText.setFillColor(sf::Color::White);
    playerText.setPosition(playerListArea.left + 10.f,
                           playerListArea.top - TITLE_HEIGHT + 10.f); // Adjust title position

    sf::Text logsTitle;
    logsTitle.setFont(*_font.get());
    logsTitle.setString("Logs:");
    logsTitle.setCharacterSize(24);
    logsTitle.setFillColor(sf::Color::White);
    logsTitle.setPosition(logsArea.left + 10.f, logsArea.top - TITLE_HEIGHT + 10.f); // Adjust title position

    while (window.isOpen())
    {
        handleEvents(window);

        window.clear(sf::Color::Black);

        // Draw player list and logs backgrounds and content
        drawPlayerList(window, playerListArea);
        drawLogs(window, logsArea);

        // Draw titles last to ensure they are not overwritten
        window.draw(playerText);
        window.draw(logsTitle);

        window.display();
    }
}

bool AdministratorPanel::isBannedIp(std::string target)
{
    auto allBannedIps = _sqlApi.getAllBannedIps();

    for (auto &ip : allBannedIps)
    {
        if (ip == target)
        {
            return true;
        }
    }
    return false;
}
