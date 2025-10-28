#include "RType.hpp"
#include "../client/KeybindManager.hpp"
#include "../client/NetworkECSMediator.hpp"
#include "../ecs/GraphicsManager.hpp"
#include "../ecs/ecs.hpp"
#include "../ecs/systems.hpp"
#include "../ecs/textBox.hpp"
#include "../transferData/opcode.hpp"
#include "../transferData/transferData.hpp"
#include "Network/NetworkManager.hpp"
#include "assetsPath.hpp"
#include "transferData/hashUtils.hpp"
#include "windowSize.hpp"
#include <SFML/Window/Keyboard.hpp>
#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>

#include <windows.h>
#endif
#include <SFML/Graphics/Font.hpp>
#include <exception>
#include <functional>
#include <iostream>

// Joystick configuration constants
const float JOYSTICK_DEADZONE = 25.0f; // Deadzone threshold (0-100)
const unsigned int JOYSTICK_ID = 0;    // Default joystick ID

bool RTypeGame::init(NetworkECSMediator med, std::function<void(const char *)> networkCb)
{
    sf::Font dummy;

    g_graphics = new GraphicsManager(med);
    // Initialize graphics
    if (!g_graphics->init("R-Type", windowWidth, windowHeight, networkCb))
    {
        std::cerr << "Failed to initialize graphics!" << std::endl;
        return false;
    }

    createTextures();
    keybindMenu = new KeybindMenu(keybindManager);

    running = true;

    // Check for connected joysticks
    checkJoystickConnection();

    std::cout << "R-Type initialized!" << std::endl;
    return true;
}

void RTypeGame::checkJoystickConnection()
{
    for (unsigned int i = 0; i < sf::Joystick::Count; ++i)
    {
        if (sf::Joystick::isConnected(i))
        {
            std::cout << "Joystick " << i << " connected!" << std::endl;
            std::cout << "Button count: " << sf::Joystick::getButtonCount(i) << std::endl;
            std::cout << "Has X axis: " << sf::Joystick::hasAxis(i, sf::Joystick::X) << std::endl;
            std::cout << "Has Y axis: " << sf::Joystick::hasAxis(i, sf::Joystick::Y) << std::endl;
        }
    }
}

void RTypeGame::createTextures()
{
    sf::Texture &backgroundTexture =
        g_graphics->createTextureFromPath(PathFormater::formatAssetPath(backgroundSpritePath), "background");
    sf::Texture &playerTexture =
        g_graphics->createTextureFromPath(PathFormater::formatAssetPath(playerSpritePath), "player");
    sf::Texture &bossTexture = g_graphics->createTextureFromPath(PathFormater::formatAssetPath(bossSpritePAth), "boss");
    sf::Texture &bulletTexture =
        g_graphics->createTextureFromPath(PathFormater::formatAssetPath(bulletSpritePath), "bullet");
    sf::Texture &basicEnemyTexture =
        g_graphics->createTextureFromPath(PathFormater::formatAssetPath(basicEnemySpritePath), "basic_enemy");
    sf::Texture &bonusLifeTexture =
        g_graphics->createTextureFromPath(PathFormater::formatAssetPath(bonusLifeSpritePath), "bonus_life");
    sf::Texture &explosionTexture =
        g_graphics->createTextureFromPath(PathFormater::formatAssetPath(explosionSpritePath), "explosion");

    g_graphics->storeTexture("background", backgroundTexture);
    g_graphics->storeTexture("boss", bossTexture);
    g_graphics->storeTexture("player", playerTexture);
    g_graphics->storeTexture("basic_enemy", basicEnemyTexture);
    g_graphics->storeTexture("bullet", bulletTexture);
    g_graphics->storeTexture("bonus_life", bonusLifeTexture);
    g_graphics->storeTexture("explosion", explosionTexture);
}

void RTypeGame::handleJoystickInput()
{
    if (!player || !player->hasComponent<InputComponent>())
        return;

    if (!sf::Joystick::isConnected(JOYSTICK_ID))
        return;

    auto &input = player->getComponent<InputComponent>();

    if (sf::Joystick::hasAxis(JOYSTICK_ID, sf::Joystick::X))
    {
        float xAxis = sf::Joystick::getAxisPosition(JOYSTICK_ID, sf::Joystick::X);

        if (xAxis < -JOYSTICK_DEADZONE)
            input.left = true;
        else if (xAxis > JOYSTICK_DEADZONE)
            input.right = true;
        else
        {
            input.left = false;
            input.right = false;
        }
    }

    if (sf::Joystick::hasAxis(JOYSTICK_ID, sf::Joystick::Y))
    {
        float yAxis = sf::Joystick::getAxisPosition(JOYSTICK_ID, sf::Joystick::Y);

        if (yAxis < -JOYSTICK_DEADZONE)
            input.up = true;
        else if (yAxis > JOYSTICK_DEADZONE)
            input.down = true;
        else
        {
            input.up = false;
            input.down = false;
        }
    }

    if (sf::Joystick::hasAxis(JOYSTICK_ID, sf::Joystick::PovX))
    {
        float povX = sf::Joystick::getAxisPosition(JOYSTICK_ID, sf::Joystick::PovX);
        if (povX < -50.0f)
            input.left = true;
        else if (povX > 50.0f)
            input.right = true;
    }

    if (sf::Joystick::hasAxis(JOYSTICK_ID, sf::Joystick::PovY))
    {
        float povY = sf::Joystick::getAxisPosition(JOYSTICK_ID, sf::Joystick::PovY);
        if (povY < -50.0f)
            input.up = true;
        else if (povY > 50.0f)
            input.down = true;
    }

    if (sf::Joystick::isButtonPressed(JOYSTICK_ID, 0))
    {
        if (!input.fire)
            g_graphics->playSound("pew");
        input.fire = true;
    }
    else
    {
        input.fire = false;
    }

    if (sf::Joystick::isButtonPressed(JOYSTICK_ID, 7))
    {
        input.enter = true;
    }
    else
    {
        input.enter = false;
    }
    // must include warp
}

void RTypeGame::handleEvents()
{
    sf::Event event;
    while (g_graphics->getWindow().pollEvent(event))
    {
        if (event.type == sf::Event::Closed)
        {
            running = false;
            std::cout << "Running is set to false" << std::endl;
        }

        if (gameOver && event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape)
        {
            running = false;
            std::cout << "Running is set to false" << std::endl;
        }

        keybindMenu->handleEvent(event, g_graphics->getWindow());

        if (_state == GameState::MENULOGIN)
        {
            if (event.type == sf::Event::MouseButtonPressed)
            {
                sf::Vector2i mousePos = sf::Mouse::getPosition(g_graphics->getWindow());
                g_graphics->getUsernameTextBox()->checkInFocus(mousePos);
                g_graphics->getPasswordTextBox()->checkInFocus(mousePos);
            }

            // Pass keyboard events to the focused textbox
            g_graphics->getUsernameTextBox()->typeInBox(event);
            g_graphics->getPasswordTextBox()->typeInBox(event);

            // Handle login menu clicks
            if (event.type == sf::Event::MouseButtonPressed)
            {
                auto mousePos = sf::Mouse::getPosition(g_graphics->getWindow());
                auto action = g_graphics->handleMenuClick(mousePos.x, mousePos.y);

                if (action == GraphicsManager::MenuAction::LOGIN)
                {
                    std::string username = g_graphics->getUsernameTextBox()->getText();
                    std::string password = g_graphics->getPasswordTextBox()->getText();
                    std::string loginData = serializeString(username) + serializeString(password);
                    std::vector<uint8_t> encryptedData;
                    unsigned char tag[16];

                    if (!aesEncryptWithTag(_med.getNetworkManager().getAesKey(), _med.getNetworkManager().getAesIV(),
                                           loginData, encryptedData))
                    {
                        std::cerr << "Client failed to encrypt data\n";
                        EVP_PKEY_free(_med.getNetworkManager().getServerPubKey());
                        return;
                    }
                    _playerName = username;
                    std::string encryptedDataStr(encryptedData.begin(), encryptedData.end());
                    _med.notify(NetworkECSMediatorEvent::SEND_DATA_TCP, encryptedDataStr, OPCODE_LOGIN_REQUEST);
                }
                else if (action == GraphicsManager::MenuAction::SIGNIN)
                {
                    std::string username = g_graphics->getUsernameTextBox()->getText();
                    std::string password = g_graphics->getPasswordTextBox()->getText();
                    std::string loginData = serializeString(username) + serializeString(password);
                    std::vector<uint8_t> encryptedData;

                    unsigned char tag[16];
                    if (!aesEncryptWithTag(_med.getNetworkManager().getAesKey(), _med.getNetworkManager().getAesIV(),
                                           loginData, encryptedData))
                    {
                        std::cerr << "Client failed to encrypt data\n";
                        EVP_PKEY_free(_med.getNetworkManager().getServerPubKey());
                        return;
                    }
                    _playerName = username;
                    std::string encryptedDataStr(encryptedData.begin(), encryptedData.end());
                    EVP_PKEY_free(_med.getNetworkManager().getServerPubKey());

                    _med.notify(NetworkECSMediatorEvent::SEND_DATA_TCP, encryptedDataStr, OPCODE_SIGNIN_REQUEST);
                }
            }
        }

        if (_state == GameState::MENUIP)
        {
            if (event.type == sf::Event::MouseButtonPressed)
                g_graphics->getTextBox()->checkInFocus(sf::Mouse::getPosition(g_graphics->getWindow()));

            g_graphics->getTextBox()->typeInBox(event);
        }

        if (_state == GameState::MENULOBBY && event.type == sf::Event::MouseButtonPressed)
        {
            auto mousePos = sf::Mouse::getPosition(g_graphics->getWindow());

            g_graphics->getLobbyTextBox()->checkInFocus(mousePos);

            auto action = g_graphics->handleLobbyMenuClick(mousePos.x, mousePos.y);

            if (action == GraphicsManager::MenuAction::CREATE_LOBBY)
            {
                std::string lobbyName = g_graphics->getLobbyTextBox()->getText();
                if (!lobbyName.empty())
                {
                    std::cout << "[Client] Creating lobby: " << lobbyName << std::endl;
                    _med.notify(NetworkECSMediatorEvent::SEND_DATA_TCP, lobbyName, OPCODE_CREATE_LOBBY);
                }
                else
                {
                    std::cout << "[Client] Lobby name cannot be empty!" << std::endl;
                }
            }
            else if (action == GraphicsManager::MenuAction::JOIN_LOBBY)
            {
                std::string lobbyName = g_graphics->getLobbyTextBox()->getText();
                if (!lobbyName.empty())
                {
                    std::string data = lobbyName + " " + _playerName;
                    std::cout << "[Client] Joining lobby: " << lobbyName << std::endl;
                    _med.notify(NetworkECSMediatorEvent::SEND_DATA_TCP, data, OPCODE_JOIN_LOBBY);
                }
                else
                {
                    std::cout << "[Client] Lobby name cannot be empty!" << std::endl;
                }
            }
            else if (action == GraphicsManager::MenuAction::BACK)
            {
                _state = GameState::MENULOGIN;
                std::cout << "[Client] Going back to IP menu" << std::endl;
            }
        }

        if (_state == GameState::MENULOBBY)
        {
            g_graphics->getLobbyTextBox()->typeInBox(event);
        }

        keybindMenu->handleEvent(event, g_graphics->getWindow());

        if (event.type == sf::Event::KeyPressed || event.type == sf::Event::KeyReleased)
        {
            bool isPressed = (event.type == sf::Event::KeyPressed);

            sf::Keyboard::Key processedKey = keybindManager.processKeyInput(event.key.code);

            if (player && player->hasComponent<InputComponent>())
            {
                auto &input = player->getComponent<InputComponent>();

                switch (processedKey)
                {
                case sf::Keyboard::Up:
                    input.up = isPressed;
                    break;
                case sf::Keyboard::Down:
                    input.down = isPressed;
                    break;
                case sf::Keyboard::Left:
                    input.left = isPressed;
                    break;
                case sf::Keyboard::Right:
                    input.right = isPressed;
                    break;
                case sf::Keyboard::Space:
                    if (isPressed)
                        g_graphics->playSound("pew");
                    input.fire = isPressed;
                    break;
                case sf::Keyboard::Enter:
                    input.enter = isPressed;
                    break;
                case sf::Keyboard::W:
                    input.warp = isPressed;
                    break;
                default:
                    break;
                }
            }
        }
    }
}

void RTypeGame::findMyPlayer()
{
    auto players = entityManager.getEntitiesWithComponent<PlayerComponent>();

    for (auto *entity : players)
    {
        auto &playerComp = entity->getComponent<PlayerComponent>();
        if (playerComp.playerID == _playerId)
        {
            player = entity;
            std::cout << "Player has been found" << std::endl;
            break;
        }
    }
}

void RTypeGame::update(float deltaTime)
{
    if (gameOver)
        return;

    tickSystem.update(entityManager);
    backgroundSystem.update(entityManager, deltaTime);
    animationSystem.update(entityManager, deltaTime);

    entityManager.applyPendingChanges();
    if (player == nullptr)
    {
        findMyPlayer();
    }

    handleJoystickInput();
}

void RTypeGame::render()
{
    g_graphics->clear();

    if (_state == GameState::KICKED)
    {
        g_graphics->getWindow().clear(sf::Color::Black);
        g_graphics->drawText("You have been kicked by an administrator.", 0, windowHeight / 2);
    }
    else if (_state == GameState::BAN)
    {
        g_graphics->getWindow().clear(sf::Color::Black);
        g_graphics->drawText("Your ip was banned by an administrator.", 0, windowHeight / 2);
    }
    else if (_state == GameState::MENULOGIN)
    {
        g_graphics->drawMenu();
        g_graphics->updateErrorMessage();
        keybindMenu->draw(g_graphics->getWindow());
    }
    else if (_state == GameState::MENUIP)
    {
        g_graphics->drawText("Type the IP to connect in the text-box:", 0, 0);
        g_graphics->drawText("SeymourPintatGodeFeytGrodard-Type", 0, 30);
        g_graphics->getTextBox()->setPosition(200, 500);
        g_graphics->getTextBox()->draw(g_graphics->getWindow());
        keybindMenu->draw(g_graphics->getWindow());
    }
    else if (_state == GameState::MENULOBBY)
    {
        g_graphics->drawLobbyMenu();
        keybindMenu->draw(g_graphics->getWindow());
    }
    else if (_state == GameState::LOBBY)
    {
        renderSystem.update(entityManager);
        drawWaitingForPlayers();
        drawTutorial();
    }
    else if (_state == GameState::INGAME)
    {
        std::string scoreText = "Score: " + std::to_string(score);
        g_graphics->drawText(scoreText, 10, 10);

        std::string waveText = "Wave: " + std::to_string(gameLogicSystem.currentWave + 1);
        g_graphics->drawText(waveText, windowWidth - 100, 10);

        drawPlayerID();
        renderSystem.update(entityManager);
    }
    else if (_state == GameState::GAMEOVER)
    {
        std::string gameOverText = "Game Over! Your score: " + std::to_string(score);
        g_graphics->drawText(gameOverText, 200, 300);
        g_graphics->drawText("Press ESC to exit", 200, 350);
    }
    drawHitbox();

    g_graphics->present();
}

void RTypeGame::restart()
{
    entityManager.clear();
    score = 0;
    gameOver = false;
}

void cleanup()
{
    if (g_graphics)
    {
        delete g_graphics;
        g_graphics = nullptr;
    }
}

void RTypeGame::sendInputPlayer()
{
    _mutex.lock();

    if (!player || !player->isActive())
    {
        _mutex.unlock();
        return;
    }

    if (player->hasComponent<InputComponent>())
    {
        auto &input = player->getComponent<InputComponent>();
        auto &playerComp = player->getComponent<PlayerComponent>();

        std::string inputData = serializePlayerInput(input, playerComp.playerID);

        _med.notify(NetworkECSMediatorEvent::SEND_DATA_UDP, inputData, OPCODE_PLAYER_INPUT);
    }

    _mutex.unlock();
}

void RTypeGame::run()
{
    const float TARGET_FPS = 60.0f;
    const float FRAME_TIME = 1.0f / TARGET_FPS;

    sf::Clock clock;
    float accumulator = 0.0f;

    g_graphics->playSound("music", true);

    std::cout << "rtype run outside\n";
    while (running)
    {
        float deltaTime = clock.restart().asSeconds();
        if (deltaTime > 0.05f)
        {
            deltaTime = 0.05f;
        }

        handleEvents();

        if (running == false)
            break;

        sendInputPlayer();

        _mutex.lock();
        update(deltaTime);
        _mutex.unlock();

        render();
    }

    cleanup();
}

void RTypeGame::drawWaitingForPlayers()
{
    if (_playerNb == 0 || _playerReady >= _playerNb)
    {
        return;
    }

    std::string waitingText =
        "Waiting for players to be ready: " + std::to_string(_playerReady) + " / " + std::to_string(_playerNb);

    int windowWidth = g_graphics->getWindow().getSize().x;
    int windowHeight = g_graphics->getWindow().getSize().y;
    float textX = windowWidth / 2.0f - 200.0f;
    float textY = windowHeight / 2.0f - 50.0f;

    g_graphics->drawText(waitingText, textX, textY);
}

void RTypeGame::drawTutorial()
{
    if (_playerNb == 0 || _playerReady >= _playerNb)
    {
        return;
    }

    std::string moveText = "Use ARROW KEYS or LEFT STICK to move";
    std::string shootText = "Press SPACE or BUTTON A to shoot";

    int windowWidth = g_graphics->getWindow().getSize().x;
    int windowHeight = g_graphics->getWindow().getSize().y;
    float textX = windowWidth / 2.0f - 150.0f;
    float textY = windowHeight / 2.0f + 10.0f;
    float textY2 = windowHeight / 2.0f + 60.0f;

    g_graphics->drawText(moveText, textX, textY);
    g_graphics->drawText(shootText, textX, textY2);
}

void RTypeGame::setCurrentWave(int nb)
{
    gameLogicSystem.currentWave = nb;
}

void RTypeGame::updateScore(std::vector<std::pair<int, int>> vec)
{
    for (auto &pair : vec)
    {
        if (pair.first == _playerId)
        {
            puts("SCORE UPDATED");
            score = pair.second;
        }
    }
}

void RTypeGame::setCurrentState(GameState newState)
{
    switch (newState)
    {
    case GameState::MENULOGIN:
        std::cout << "In MENU state" << std::endl;
        break;
    case GameState::MENUIP:
        std::cout << "In MENUIP state" << std::endl;
        break;
    case GameState::MENULOBBY:
        std::cout << "In MENULOBBY state" << std::endl;
        break;
    case GameState::INGAME:
        std::cout << "In INGAME state" << std::endl;
        break;
    case GameState::GAMEOVER:
        std::cout << "In GAMEOVER state" << std::endl;
        break;
    default:
        std::cout << "In UNKNOWN state" << std::endl;
        break;
    }
    _state = newState;
}

void RTypeGame::drawHitbox()
{
    auto entities = entityManager.getEntitiesWithComponent<ColliderComponent>();
    for (auto e : entities)
    {
        auto &collider = e->getComponent<ColliderComponent>();

        if (!collider.isActive)
            continue;

        sf::RectangleShape hitboxRect;

        hitboxRect.setSize(sf::Vector2f(collider.hitbox.w, collider.hitbox.h));
        hitboxRect.setOrigin(sf::Vector2f(collider.hitbox.w / 2, collider.hitbox.h / 2));

        if (e->hasComponent<TransformComponent>())
        {
            auto &transform = e->getComponent<TransformComponent>();
            hitboxRect.setPosition(transform.position.x, transform.position.y);
        }
        else
        {
            hitboxRect.setPosition(collider.hitbox.x, collider.hitbox.y);
        }

        hitboxRect.setFillColor(sf::Color(0, 0, 0, 0));
        hitboxRect.setOutlineThickness(1.0f);
        hitboxRect.setOutlineColor(sf::Color::Red);

        g_graphics->getWindow().draw(hitboxRect);
    }
}

void RTypeGame::drawPlayerID()
{
    auto entities = entityManager.getEntitiesWithComponent<PlayerComponent>();
    for (auto e : entities)
    {
        auto &player = e->getComponent<PlayerComponent>();

        std::string username = "PLAYER " + std::to_string(player.playerID);

        if (_playerId == player.playerID)
            username = "ME";

        if (e->hasComponent<TransformComponent>())
        {
            auto &transform = e->getComponent<TransformComponent>();
            g_graphics->drawText(username, transform.position.x, transform.position.y, 255, 255, 255);
        }
    }
}

void RTypeGame::markPlayerAsDead(int playerId)
{
    auto players = entityManager.getEntitiesWithComponent<PlayerComponent>();
    for (auto *entity : players)
    {
        auto &playerComp = entity->getComponent<PlayerComponent>();
        if (playerComp.playerID == playerId)
        {
            if (entity->hasComponent<HealthComponent>())
            {
                auto &healthComp = entity->getComponent<HealthComponent>();
                healthComp.health = 0;
                std::cout << "Player " << playerId << " marked as dead." << std::endl;
            }
            break;
        }
    }
}

void RTypeGame::setWinnerId(int id)
{
    _winnerId = id;

    std::cout << "Game Over!, set winner ID to " << _winnerId << std::endl;
    if (_winnerId == _playerId)
    {
        std::cout << "You are the winner!" << std::endl;
    }
    else if (_winnerId == -1)
    {
        std::cout << "It's a draw! No winners." << std::endl;
    }
    else
    {
        std::cout << "Player " << _winnerId << " is the winner!" << std::endl;
    }
    _state = GameState::GAMEOVER;
}