#include "../client/KeybindManager.hpp"
#include "../ecs/IComponent.hpp"
#include "../ecs/keybindMenu.hpp"
#include "../ecs/systems.hpp"
#include "../ecs/textBox.hpp"
#include "NetworkECSMediator.hpp"
#include "VoiceManager.hpp"
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
#include <SFML/Window/Keyboard.hpp>
#include <mutex>
#include <unordered_map>

#pragma once

enum GameState
{
    MENUIP,
    MENULOBBY,
    LOBBY,
    INGAME,
    MENU,
    GAMEOVER,
    KICKED,
    BAN,
};

class RTypeGame
{
  private:
    int _playerId;
    std::mutex _mutex;
    EntityManager entityManager;

    // Systems
    MovementSystem movementSystem;
    RenderSystem renderSystem;
    CollisionSystem collisionSystem;
    BoundarySystem boundarySystem;        // Generic boundary checking
    OffscreenCleanupSystem cleanupSystem; // Generic cleanup
    InputSystem inputSystem;              // Use existing generic input system
    PlayerSystem playerSystem;
    AnimationSystem animationSystem;
    EnemySystem enemySystem;
    LaserWarningSystem laserWarningSystem;
    GameLogicSystem gameLogicSystem;
    BackgroundSystem backgroundSystem;

    NetworkECSMediator &_med;

    Entity *player = nullptr;
    bool gameOver = false;

    bool running = false;

    int _playerReady = 0;
    int _playerNb = 0;
    int _winnerId = -1;
    bool showLobbyInfo = false;

    GameState _state = GameState::MENU;

    KeybindManager keybindManager;
    KeybindMenu *keybindMenu;

    int score = 0;

    std::function<void(const char *)> _networkCb;

    const float ENEMY_SPEED = -200.0f;

    std::vector<AudioDevice> _availableMicrophones;
    int _selectedMicrophoneIndex = -1;
    bool _showMicrophoneMenu = false;
    std::vector<sf::FloatRect> _microphoneMenuButtons;

  public:
    void reset();

    void toggleMicrophoneMenu()
    {
        _showMicrophoneMenu = !_showMicrophoneMenu;
    }
    void selectMicrophone(int index);
    bool handleMicrophoneMenuClick(int mouseX, int mouseY);
    void drawMicrophoneMenu();
    std::vector<AudioDevice> &getAvailableMicrophones()
    {
        return _availableMicrophones;
    }
    void loadAvailableMicrophones();

    int getSelectedMic()
    {
        return _selectedMicrophoneIndex;
    };

    RTypeGame(NetworkECSMediator &med) : _med(med){};

    void markPlayerAsDead(int playerId);

    void handleJoystickInput();
    void checkJoystickConnection();
    void setWinnerId(int id);

    void setGameOver(bool value)
    {
        gameOver = value;
    }

    std::mutex &getMutex()
    {
        return _mutex;
    }

    void findMyPlayer();

    EntityManager &getEntityManager()
    {
        return entityManager;
    }
    void setPlayerId(int id)
    {
        _playerId = id;
    };
    void setPlayerReady(int value)
    {
        _playerReady = value;
    }
    void setPlayerNb(int value)
    {
        _playerNb = value;
    }

    bool init(NetworkECSMediator med, std::function<void(const char *)> networkCb);

    void createTextures();

    void createBackground();

    void createPlayer();

    void drawWaitingForPlayers();
    void handleEvents();
    // void handleEvents();

    void sendInputPlayer();
    void drawTutorial();

    void update(float deltaTime);

    void render();

    void restart();

    void run();

    void setCurrentWave(int nb);
    void updateScore(std::vector<std::pair<int, int>> vec);

    void setCurrentState(GameState newState);

    void drawHitbox();
    void drawPlayerID();

    void setKickState()
    {
        _state = GameState::KICKED;
    };

    void setBanState()
    {
        _state = GameState::BAN;
    };
};