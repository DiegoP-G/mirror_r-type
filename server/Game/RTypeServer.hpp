#pragma once
#include "../../ecs/entity.hpp"
#include "../../ecs/entityManager.hpp"
#include "../../ecs/systems.hpp"
#include "../../transferData/transferData.hpp"
#include "GameMediator.hpp"
#include <iostream>
#include <random>

class GameMediator;

enum GameState
{
    MENUIP,
    MENULOBBY,
    LOBBY,
    INGAME,
    MENU,
    GAMEOVER
};

class RTypeServer
{
  private:
    GameMediator &mediator;
    EntityManager entityManager;

    MovementSystem movementSystem;
    RenderSystem renderSystem;
    CollisionSystem collisionSystem;
    BoundarySystem boundarySystem;
    OffscreenCleanupSystem cleanupSystem;
    InputSystem inputSystem;
    PlayerSystem playerSystem;
    EnemySystem enemySystem;
    LaserWarningSystem laserWarningSystem;
    GameLogicSystem gameLogicSystem;
    BackgroundSystem backgroundSystem;
    ProjectileSystem projectileSystem;
    BonusSystem bonusSystem;

    Entity *player = nullptr;
    bool gameOver = false;
    GameState _state = LOBBY;

    bool running = false;

    int playerReady = 0;
    int playerNb = 0;

    int score = 0;
    int tick = 0;

    std::string _lobbyUID;
    const float ENEMY_SPEED = -200.0f;

    // Player scores: PlayerId - Score
    std::vector<std::pair<int, int>> _playersScores;

  public:
    RTypeServer(GameMediator &mediator, std::string &lobbyUID) : mediator(mediator), _lobbyUID(lobbyUID)
    {
        std::cout << "ffff" << std::endl;
        std::cout << _lobbyUID << std::endl;
    }

    void removePlayer(int id);

    std::vector<std::string> serializeAllActiveEntities()
    {
        std::vector<std::string> result;

        // Parcourir toutes les entités actives
        for (auto &entity : entityManager.getEntities())
        {
            if (entity)
            {
                auto data = entityManager.serializeEntityFull(entity->getID());
                if (!data.empty())
                {
                    std::string serializedData(data.begin(), data.end());
                    result.push_back(serializedData);
                }
            }
        }

        std::cout << "[RTypeServer] Serialized " << result.size() << " active entities\n";
        return result;
    }
    void sendMovementUpdates();
    void sendHealthUpdates();
    void sendNewEntities();
    void sendDestroyedEntities();
    void createBackground();
    void sendGameStateUpdates();

    void updateLobbyStatus();

    Entity *getEntityByPlayerID(int playerID);

    bool init();

    void createPlayer(int clientFd);

    void update(float deltaTime);

    void restart();

    void cleanup();

    void run(float deltaTime);

    void sendEntities();

    void handlePlayerInput(const std::string &);
};