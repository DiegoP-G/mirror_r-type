#pragma once
#include "../ecs/ecs.hpp"
#include "../ecs/systems.hpp"
#include <cstdint>
#include <deque>
#include <vector>

// ============ STRUCTURES ============

struct PlayerInput
{
    uint32_t tickNumber;
    uint32_t sequenceNumber;
    bool moveUp, moveDown, moveLeft, moveRight;
    bool shoot;
};

struct PlayerSnapshot
{
    int entityId;
    float x, y, vx, vy;
    int health, score, ammo;
};

struct ProjectileSnapshot
{
    int entityId;
    float x, y, vx, vy;
};

struct GameSnapshot
{
    uint32_t tickNumber;
    float timestamp;
    bool valid = true;
    std::vector<PlayerSnapshot> players;
    std::vector<ProjectileSnapshot> projectiles;
};

// ============ TICK SYSTEM ============

class TickSystem
{
  private:
    // Configuration
    static constexpr float TICK_DURATION = 0.0165f; // 16.5ms = ~60 ticks/sec
    static constexpr size_t MAX_SNAPSHOTS = 120;    // Garde 2 secondes d'historique

    // État
    float tickAccumulator = 0.0f;
    uint32_t currentTick = 0;
    uint32_t lastConfirmedServerTick = 0;
    uint32_t lastProcessedInputSequence = 0;
    int _playerId = 0;

    // Historique
    std::deque<GameSnapshot> snapshotHistory;
    std::deque<PlayerInput> inputHistory;

    MovementSystem movementSystem;
    ProjectileSystem projectileSystem;
    CollisionSystem collisionSystem;
    BoundarySystem boundarySystem;
    OffscreenCleanupSystem cleanupSystem;
    InputSystem inputSystem;
    PlayerSystem playerSystem;
    EnemySystem enemySystem;
    LaserWarningSystem laserWarningSystem;
    GameLogicSystem gameLogicSystem;

    // Méthodes privées pour la simulation client
    PlayerInput capturePlayerInput(EntityManager &entityManager);
    void applyPlayerInputs(EntityManager &entityManager, const PlayerInput &input);
    void simulatePlayerOnly(EntityManager &entityManager);
    void createPlayerProjectile(EntityManager &entityManager, Entity *player);

    // Méthodes privées pour le rollback
    void restorePlayerFromSnapshot(Entity *player, const PlayerSnapshot &snap);

  public:
    bool predictionEnabled = false;

    // ============ MÉTHODES PRINCIPALES ============

    // Appelé chaque frame côté client
    void update(EntityManager &entityManager);

    // Traite un tick de jeu (CLIENT: uniquement joueur)
    void processTick(EntityManager &entityManager);

    // Quand on reçoit la confirmation du serveur
    void onServerUpdate(uint32_t serverTick, EntityManager &serverEM, EntityManager &clientEm);

    // ============ MÉTHODES DE SNAPSHOT ============

    void saveSnapshot(EntityManager &entityManager);
    GameSnapshot getSnapshot(uint32_t tick);

    // ============ MÉTHODES DE COMPARAISON/ROLLBACK ============

    bool statesDiffer(GameSnapshot &clientSnapshot, EntityManager &serverEM);

    void performRollback(uint32_t serverTick, EntityManager &serverEM, EntityManager &clientEm);

    void forceStateUpdate(EntityManager &clientEm, EntityManager &serverEM);

    // ============ UTILITAIRES ============

    void cleanupHistory();

    uint32_t getCurrentTick() const
    {
        return currentTick;
    }
    float getTickDuration() const
    {
        return TICK_DURATION;
    }
    float getTickProgress() const
    {
        return tickAccumulator / TICK_DURATION;
    }

    // Accès aux systems (pour le serveur principalement)
    GameLogicSystem &getGameLogicSystem()
    {
        return gameLogicSystem;
    }

    // Pour debug
    size_t getSnapshotHistorySize() const
    {
        return snapshotHistory.size();
    }
    size_t getInputHistorySize() const
    {
        return inputHistory.size();
    }

    void setPlayerId(int id)
    {
        _playerId = id;
    };
    int lastServerTick = 0;
};