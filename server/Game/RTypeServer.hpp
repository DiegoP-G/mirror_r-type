#pragma once
#include "../../ecs/entity.hpp"
#include "../../ecs/entityManager.hpp"
#include "../../ecs/systems.hpp"
#include "../../transferData/transferData.hpp"
#include "GameMediator.hpp"
#include <iostream>
#include <random>

class GameMediator;

enum GameState {
    LOBBY,
    INGAME
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

    Entity *player = nullptr;
    bool gameOver = false;
    GameState _state = LOBBY;

    bool running = false;

    int playerReady = 0;
    int playerNb = 0;

    int score = 0;
    int tick = 0;

    const float ENEMY_SPEED = -200.0f;

  public:
    RTypeServer(GameMediator &mediator) : mediator(mediator)
    {
    }

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

    void updateLobbyStatus();

    Entity *getEntityByPlayerID(int playerID);

    bool init();

    void createPlayer(const std::string &);

    void update(float deltaTime);

    void restart();

    void cleanup();

    void run(float deltaTime);

    void sendEntities();

    void handlePlayerInput(const std::string &);
};