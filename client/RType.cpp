#include "RType.hpp"
#include "../client/KeybindManager.hpp"
#include "../client/NetworkECSMediator.hpp"
#include "../ecs/GraphicsManager.hpp"
#include "../ecs/ecs.hpp"
#include "../ecs/systems.hpp"
#include "../ecs/textBox.hpp"
#include "../transferData/opcode.hpp"
#include "Network/NetworkManager.hpp"
#include "assetsPath.hpp"
#include "windowSize.hpp"
#include <SFML/Graphics/Font.hpp>
#include <exception>
#include <functional>
#include <iostream>

// void ClientGame::startServer(const char *serverIp)
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

    std::cout << "R-Type initialized!" << std::endl;
    return true;
}

void RTypeGame::createTextures()
{
    sf::Texture &backgroundTexture =
        g_graphics->createTextureFromPath(PathFormater::formatAssetPath(backgroundSpritePath), "background");
    sf::Texture &playerTexture =
        g_graphics->createTextureFromPath(PathFormater::formatAssetPath(playerSpritePath), "player");
    sf::Texture &explosionTexture =
        g_graphics->createTextureFromPath(PathFormater::formatAssetPath("assets/sprites/explosion.png"), "explosion");

    sf::Texture &bulletTexture =
        g_graphics->createTextureFromPath(PathFormater::formatAssetPath(bulletSpritePath), "bullet");
    sf::Texture &basicEnemyTexture =
        g_graphics->createTextureFromPath(PathFormater::formatAssetPath(basicEnemySpritePath), "basic_enemy");
    sf::Texture &bonusLifeTexture =
        g_graphics->createTextureFromPath(PathFormater::formatAssetPath(bonusLifeSpritePath), "bonus_life");

    g_graphics->storeTexture("background", backgroundTexture);
    g_graphics->storeTexture("player", playerTexture);
    g_graphics->storeTexture("basic_enemy", basicEnemyTexture);
    g_graphics->storeTexture("bullet", bulletTexture);
    g_graphics->storeTexture("explosion", explosionTexture);
    g_graphics->storeTexture("bonus_life", bonusLifeTexture);
}

void RTypeGame::handleEvents()
{
    sf::Event event;
    while (g_graphics->getWindow().pollEvent(event))
    {
        if (event.type == sf::Event::Closed)
        {
            running = false;
        }

        if (event.type == sf::Event::MouseButtonPressed)
            g_graphics->getTextBox()->checkInFocus(sf::Mouse::getPosition(g_graphics->getWindow()));

        g_graphics->getTextBox()->typeInBox(event);
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
                    g_graphics->playSound("pew");
                    input.fire = isPressed;
                    break;
                case sf::Keyboard::Enter:
                    input.enter = isPressed;
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
    // std::cout << "[CLIENT] Looking for player with ID: " << _playerId <<
    // std::endl; std::cout << "[CLIENT] Found " << players.size() << " entities
    // with PlayerComponent" << std::endl;

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

    // Update systems
    // gameLogicSystem.update(entityManager, deltaTime);
    //   backgroundSystem.update(entityManager, deltaTime);
    // movementSystem.update(entityManager, deltaTime);
    // playerSystem.update(entityManager, deltaTime);
    animationSystem.update(entityManager, deltaTime);
    // inputSystem.update(entityManager, deltaTime);
    // boundarySyste>m.update(entityManager, deltaTime);
    // cleanupSystem.update(entityManager, deltaTime);
    // enemySystem.update(entityManager, deltaTime);
    // collisionSystem.update(entityManager);
    // laserWarningSystem.update(entityManager, deltaTime);
    // // Check game over
    // if (player && !player->isActive())
    // {
    //     gameOver = true;
    // }
    entityManager.applyPendingChanges();
    // std::cout << "UPT END\n";
    if (player == nullptr)
    {
        // std::cout << "Player not found, searching...\n";
        findMyPlayer();
    }
}

void RTypeGame::render()
{

    g_graphics->clear();

    // Render all entities
    renderSystem.update(entityManager);

    if (gameOver)
    {
        g_graphics->drawText("Game Over!", 250, 250);
    }

    if (_state == GameState::MENU)
    { // STATE HERE WHEN NOT CONNECTED TO THE SERVER
        g_graphics->drawText("Type the IP to connect in the text-box:", 0, 0);
        g_graphics->drawText("SeymourPintatGodeFeytGrodard-Type", 0, 30);
        g_graphics->getTextBox()->setAtCenter(g_graphics->getWindow());
        g_graphics->getTextBox()->draw(g_graphics->getWindow());
        keybindMenu->draw(g_graphics->getWindow());
    }
    else
    {
        // drawHitbox();
        std::string scoreText = "Score: " + std::to_string(score);
        g_graphics->drawText(scoreText, 10, 10);

        std::string waveText = "Wave: " + std::to_string(gameLogicSystem.currentWave + 1);
        g_graphics->drawText(waveText, windowWidth - 100, 10);
    }

    drawWaitingForPlayers();
    drawTutorial();
    g_graphics->present();
}

void RTypeGame::restart()
{
    entityManager = EntityManager();
    score = 0;
    gameOver = false;

    // createPlayer();
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

        // std::cout << "Sent player input for player " << playerComp.playerID <<
        // std::endl;
    }

    _mutex.unlock();
}

void RTypeGame::run()
{
    // if (!init(_med))
    // {
    //     return;
    // }

    const float TARGET_FPS = 60.0f;
    const float FRAME_TIME = 1.0f / TARGET_FPS;

    sf::Clock clock;
    float accumulator = 0.0f;

    g_graphics->playSound("music", true);
    while (running)
    {
        float deltaTime = clock.restart().asSeconds();

        // Cap delta time to prevent large jumps
        if (deltaTime > 0.05f)
        {
            deltaTime = 0.05f;
        }

        accumulator += deltaTime;
        handleEvents();
        // handleEvents();

        sendInputPlayer();

        // Fixed timestep update
        while (accumulator >= FRAME_TIME)
        {
            _mutex.lock();
            update(FRAME_TIME);
            _mutex.unlock();
            accumulator -= FRAME_TIME;
        }

        render();
        // render();
    }

    cleanup();
}

void RTypeGame::createBackground()
{
    // sf::Texture *backgroundTexture = g_graphics->getTexture("background");
    int tileWidth = 800;
    int tileHeight = 600;

    auto createBackgroundEntity = [&](float x) -> Entity & {
        auto &backgroundEntity = entityManager.createEntity();

        backgroundEntity.addComponent<TransformComponent>(x, 0.0f, 1.0f, 1.0f, 0.0f);
        backgroundEntity.addComponent<SpriteComponent>(tileWidth, tileHeight, 255, 255, 255,
                                                       GraphicsManager::Texture::BACKGROUND);
        backgroundEntity.addComponent<BackgroundScrollComponent>(-300.0f, true);

        return backgroundEntity;
    };

    createBackgroundEntity(0.0f);
    createBackgroundEntity((float)tileWidth);
}

void RTypeGame::drawWaitingForPlayers()
{
    // Format the waiting text
    if (_playerNb == 0 || _playerReady >= _playerNb)
    {

        return;
    }

    std::string waitingText =
        "Waiting for players to be ready: " + std::to_string(_playerReady) + " / " + std::to_string(_playerNb);

    // Choose a screen position (centered horizontally)
    int windowWidth = g_graphics->getWindow().getSize().x;
    int windowHeight = g_graphics->getWindow().getSize().y;
    float textX = windowWidth / 2.0f - 200.0f; // Adjust for centering
    float textY = windowHeight / 2.0f - 50.0f;

    // Draw the text on screen
    g_graphics->drawText(waitingText, textX, textY);
}

void RTypeGame::drawTutorial()
{
    if (_playerNb == 0 || _playerReady >= _playerNb)
    {

        return;
    }

    std::string moveText = "Use ARROW KEYS to move";
    std::string shootText = "Press SPACE to shoot";

    int windowWidth = g_graphics->getWindow().getSize().x;
    int windowHeight = g_graphics->getWindow().getSize().y;
    float textX = windowWidth / 2.0f - 100.0f;
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
    _state = newState;
}

void RTypeGame::drawHitbox()
{
    // --- Debug: draw hitboxes ---
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
            // Default position (just in case)
            hitboxRect.setPosition(collider.hitbox.x, collider.hitbox.y);
        }

        hitboxRect.setFillColor(sf::Color(0, 0, 0, 0)); // transparent
        hitboxRect.setOutlineThickness(1.0f);
        hitboxRect.setOutlineColor(sf::Color::Red);

        g_graphics->getWindow().draw(hitboxRect);
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
                healthComp.health = 0; // Set health to 0 to mark as dead
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
    sf::sleep(sf::seconds(3));
    g_graphics->getWindow().close();
}
