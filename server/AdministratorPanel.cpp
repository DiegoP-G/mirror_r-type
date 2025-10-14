#include "AdministratorPanel.hpp"
#include "Network/ClientManager.hpp"
#include <arpa/inet.h>

AdministratorPanel::AdministratorPanel(NetworkManager &networkManager)
    : _clientManager(nullptr), _playerListScrollOffset(0), _logsScrollOffset(0), _networkManager(networkManager)
{
    sf::Font font;
    if (!font.loadFromFile(PathFormater::formatAssetPath(fontPath)))
    {
        std::cerr << "Failed to load font: " << fontPath << std::endl;
        _font = nullptr;
        return;
    }
    _font = std::make_unique<sf::Font>(font);
}

void AdministratorPanel::setClientManager(ClientManager &clientManager)
{
    _clientManager = &clientManager;
}

void AdministratorPanel::scrollLogic(sf::RenderWindow &window, sf::Event &event)
{
    sf::Vector2i mousePos = sf::Mouse::getPosition(window);
    if (mousePos.y < WINDOWHEIGTH * 0.7f)
    {
        // Scroll the player list
        _playerListScrollOffset += event.mouseWheelScroll.delta * 20;      // Adjust scroll speed
        _playerListScrollOffset = std::max(_playerListScrollOffset, 0.0f); // Prevent scrolling above the top
    }
    else
    {
        // Scroll the logs
        _logsScrollOffset += event.mouseWheelScroll.delta * 20; // Adjust scroll speed
        _logsScrollOffset = std::max(_logsScrollOffset, 0.0f);  // Prevent scrolling above the top
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
                _networkManager.getTCPManager().sendMessage(id, OPCODE_KICK_NOTIFICATION, "");
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
        }
    }
}

std::vector<std::pair<sf::Text, sf::RectangleShape>> AdministratorPanel::buildPlayerList(
    sf::RenderWindow &window, const sf::FloatRect &playerListArea)
{
    std::vector<std::pair<sf::Text, sf::RectangleShape>> playerList;
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
        kickButton.setPosition(playerListArea.left + playerListArea.width - 120.f, y);

        // Check if the mouse is over the button
        if (kickButton.getGlobalBounds().contains(static_cast<sf::Vector2f>(mousePos)))
        {
            kickButton.setFillColor(sf::Color::White);
        }
        else
        {
            kickButton.setFillColor(sf::Color::Red);
        }

        // Store the button for event handling
        _kickButtons[id] = kickButton;

        playerList.emplace_back(playerText, kickButton);

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

    // Build and draw all players and their buttons
    std::vector<std::pair<sf::Text, sf::RectangleShape>> playerList = buildPlayerList(window, playerListArea);
    for (auto &[player, button] : playerList)
    {
        window.draw(player);
        window.draw(button);
    }

    // Draw button labels
    for (auto &[id, button] : _kickButtons)
    {
        sf::Text buttonText;
        buttonText.setFont(*_font.get());
        buttonText.setString("Kick");
        buttonText.setCharacterSize(18);
        buttonText.setFillColor(sf::Color::Black); // Text color (black for contrast on white highlight)
        buttonText.setPosition(button.getPosition().x + 10.f, button.getPosition().y + 5.f);
        window.draw(buttonText);
    }
}

void AdministratorPanel::drawLogs(sf::RenderWindow &window, sf::FloatRect logsArea)
{
    // Draw the logs area
    sf::RectangleShape logsBackground(sf::Vector2f(logsArea.width, logsArea.height));
    logsBackground.setPosition(logsArea.left, logsArea.top);
    logsBackground.setFillColor(sf::Color(30, 30, 30)); // Darker gray background
    window.draw(logsBackground);

    // Draw logs
    float logY = logsArea.top + 40 - _logsScrollOffset; // Apply scroll offset
    for (const auto &log : _logs)
    {
        // Skip drawing if the log is outside the visible area
        if (logY + 20 < logsArea.top || logY > logsArea.top + logsArea.height)
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
