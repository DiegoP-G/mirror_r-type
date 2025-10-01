#include "RType.hpp"
#include "../client/NetworkECSMediator.hpp"
#include "../ecs/GraphicsManager.hpp"
#include "../ecs/ecs.hpp"
#include "../ecs/systems.hpp"
#include "../transferData/opcode.hpp"
#include "assetsPath.hpp"
#include "windowSize.hpp"
#include <iostream>

bool RTypeGame::init(NetworkECSMediator med)
{
    _med = med;
    g_graphics = new GraphicsManager(med);
    // Initialize graphics
    if (!g_graphics->init("R-Type", windowWidth, windowHeight))
    {
        std::cerr << "Failed to initialize graphics!" << std::endl;
        return false;
    }

    // Create textures
    createTextures();

    // Create player
    createPlayer();

    running = true;

    std::cout << "R-Type initialized!" << std::endl;
    return true;
}

void RTypeGame::createTextures()
{
    sf::Texture &playerTexture = g_graphics->createTextureFromPath(PathFormater::formatAssetPath(playerSpritePath),
                                                                   "player");       // Yellow player
    sf::Texture &enemyTexture = g_graphics->createColorTexture(80, 400, 0, 255, 0); // Green enemy
    sf::Texture &bulletTexture =
        g_graphics->createTextureFromPath(PathFormater::formatAssetPath(bulletSpritePath), "bullet");
    sf::Texture &basicEnemyTexture =
        g_graphics->createTextureFromPath(PathFormater::formatAssetPath(basicEnemySpritePath), "basic_enemy");

    g_graphics->storeTexture("player", playerTexture);
    g_graphics->storeTexture("enemy", enemyTexture);
    g_graphics->storeTexture("bullet", bulletTexture);
    g_graphics->storeTexture("basic_enemy", basicEnemyTexture);
}

void RTypeGame::createPlayer()
{
    auto &playerEntity = entityManager.createEntity();

    playerEntity.addComponent<PlayerComponent>();
    playerEntity.addComponent<TransformComponent>(100.0f, 300.0f);
    playerEntity.addComponent<VelocityComponent>(0.0f, 0.0f);
    playerEntity.addComponent<SpriteComponent>(32, 32, 255, 255, 0); // Yellow
    playerEntity.addComponent<ColliderComponent>(32.0f, 32.0f);
    playerEntity.addComponent<InputComponent>();

    // Load texture and initialize AnimatedPlayerSpriteComponent
    sf::Texture *playerTexture = g_graphics->getTexture("player");
    if (playerTexture)
    {
        playerEntity.addComponent<AnimatedPlayerSpriteComponent>(*playerTexture, 33, 17.5, 0.05f, Vector2D(2.0f, 2.0f));
    }

    player = &playerEntity;
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

        if (event.type == sf::Event::KeyPressed || event.type == sf::Event::KeyReleased)
        {
            if (event.type == sf::Event::KeyPressed || event.type == sf::Event::KeyReleased)
            {
                bool isPressed = (event.type == sf::Event::KeyPressed);
                if (player && player->hasComponent<InputComponent>())
                {
                    auto &input = player->getComponent<InputComponent>();
                    switch (event.key.code)
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
                        input.fire = isPressed;
                        break;
                    default:
                        break;
                    }
                }
            }
        }
    }
}

void RTypeGame::update(float deltaTime)
{
    if (gameOver)
        return;

    // Update systems
    gameLogicSystem.update(entityManager, deltaTime);
    movementSystem.update(entityManager, deltaTime);
    playerSystem.update(entityManager, deltaTime);
    inputSystem.update(entityManager, deltaTime);
    boundarySystem.update(entityManager, deltaTime);
    cleanupSystem.update(entityManager, deltaTime);
    enemySystem.update(entityManager, deltaTime);
    waveSystem.update(entityManager, deltaTime);
    collisionSystem.update(entityManager);
    laserWarningSystem.update(entityManager, deltaTime);
    // Check game over
    if (player && !player->isActive())
    {
        gameOver = true;
    }

    entityManager.refresh();
}

void RTypeGame::render()
{
    g_graphics->clear();

    // Render all entities
    renderSystem.update(entityManager);

    if (gameOver)
    {
        g_graphics->drawText("Game Over! Press SPACE to restart", 250, 250);
    }

    std::string scoreText = "Score: " + std::to_string(score);
    g_graphics->drawText(scoreText, 10, 10);

    std::string waveText = "Wave: " + std::to_string(waveSystem.currentWave + 1);
    g_graphics->drawText(waveText, windowWidth - 100, 10);

    g_graphics->present();
}

void RTypeGame::restart()
{
    entityManager = EntityManager();

    score = 0;
    gameOver = false;

    createPlayer();
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
    if (player && player->hasComponent<InputComponent>())
    {
        auto &input = player->getComponent<InputComponent>();

        if (input.down == 1 || input.fire == 1 || input.right == 1 || input.left == 1 | input.up == 1)
        {
            std::string serializedData = serializePlayerInput(input, player->getID());
            // std::cout << "sending " << serializedData << std::endl;
            _med.notify(NetworkECSMediatorEvent::SEND_DATA_TCP, serializedData, OPCODE_PLAYER_STATE);
        }
    }
}

void RTypeGame::run()
{
    if (!init(_med))
    {
        return;
    }

    const float TARGET_FPS = 60.0f;
    const float FRAME_TIME = 1.0f / TARGET_FPS;

    sf::Clock clock;
    float accumulator = 0.0f;

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

        sendInputPlayer();

        // Fixed timestep update
        while (accumulator >= FRAME_TIME)
        {
            update(FRAME_TIME);
            accumulator -= FRAME_TIME;
        }

        render();
    }

    cleanup();
}