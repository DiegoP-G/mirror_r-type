#pragma once
#include "../ecs/ecs.hpp"
#include "../ecs/systems.hpp"
#include <cstdint>
#include <deque>
#include <iostream>
#include <memory>
#include <vector>

struct EntitySnapshot
{
    int entityId;
    float x, y;
    float velocityX, velocityY;
    int health;
    bool isActive;
    // Ajoute tous les composants nécessaires pour ton jeu
};

// Structure pour stocker l'état complet du jeu à un tick
struct GameSnapshot
{
    uint32_t tickNumber;
    float timestamp;
    std::vector<EntitySnapshot> entities;
    // Ajoute l'état de tous les systèmes importants
};

class TickSystem
{
  private:
    static constexpr float TICK_DURATION = 0.0165f; // 16.5ms = ~60 ticks/sec
    static constexpr size_t MAX_SNAPSHOTS = 120;    // Garde 2 secondes d'historique

    float tickAccumulator = 0.0f;
    uint32_t currentTick = 0;
    uint32_t lastConfirmedServerTick = 0;

    // Systems
    MovementSystem movementSystem;
    CollisionSystem collisionSystem;
    BoundarySystem boundarySystem;
    OffscreenCleanupSystem cleanupSystem;
    InputSystem inputSystem;
    PlayerSystem playerSystem;
    AnimationSystem animationSystem;
    EnemySystem enemySystem;
    LaserWarningSystem laserWarningSystem;
    GameLogicSystem gameLogicSystem;
    BackgroundSystem backgroundSystem;

    std::deque<GameSnapshot> snapshotHistory;

    // Prédiction client
    bool predictionEnabled = true;
    uint32_t lastProcessedInputSequence = 0;

  public:
    // // Appelé chaque frame
    void update(float deltaTime, EntityManager &entityManager);

    // Traite un tick de jeu
    void processTick(EntityManager &entityManager);

    // Quand on reçoit la confirmation du serveur
    void onServerUpdate(uint32_t serverTick, const std::vector<EntitySnapshot> &serverState);
    void saveSnapshot(EntityManager &entityManager);

    bool statesDiffer(const GameSnapshot &clientSnapshot, const std::vector<EntitySnapshot> &serverState);

    void performRollback(uint32_t serverTick, const std::vector<EntitySnapshot> &serverState);
    void restoreState(const std::vector<EntitySnapshot> &state);

    void simulateGameLogic(EntityManager &entityManager);

    void forceStateUpdate(const std::vector<EntitySnapshot> &serverState);

    void cleanupHistory();

  public:
    uint32_t getCurrentTick() const
    {
        return currentTick;
    }
    float getTickDuration() const
    {
        return TICK_DURATION;
    }

    // Pour l'interpolation visuelle (optionnel)
    float getTickProgress() const
    {
        return tickAccumulator / TICK_DURATION;
    }
    GameLogicSystem &getGameLogicSystem()
    {
        return gameLogicSystem;
    };
};
