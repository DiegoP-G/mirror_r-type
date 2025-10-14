#include "TickSystem.hpp"
#include "../ecs/ecs.hpp"
#include "../ecs/systems.hpp"
#include <cstdint>
#include <deque>
#include <iostream>
#include <memory>
#include <vector>

void TickSystem::update(float deltaTime, EntityManager &entityManager)
{
    tickAccumulator += deltaTime;

    while (tickAccumulator >= TICK_DURATION)
    {
        processTick(entityManager);
        tickAccumulator -= TICK_DURATION;
    }
}

void TickSystem::processTick(EntityManager &entityManager)
{
    currentTick++;

    // 1. Capture les inputs du joueur pour ce tick
    // PlayerInput input = capturePlayerInput();
    // input.tickNumber = currentTick;
    // input.sequenceNumber = lastProcessedInputSequence++;
    // inputHistory.push_back(input);

    // 2. Applique les inputs et simule le jeu
    simulateGameLogic(entityManager);

    // 3. Sauvegarde un snapshot de l'état
    saveSnapshot(entityManager);

    // 4. Nettoie l'historique trop ancien
    cleanupHistory();
}

// Quand on reçoit la confirmation du serveur
void TickSystem::onServerUpdate(uint32_t serverTick, const std::vector<EntitySnapshot> &serverState)
{
    lastConfirmedServerTick = serverTick;

    // Trouve le snapshot correspondant dans l'historique
    // auto it = std::find_if(snapshotHistory.begin(), snapshotHistory.end(),
    //     [serverTick](const GameSnapshot& snap) {
    //         return snap.tickNumber == serverTick;
    //     });

    // if (it == snapshotHistory.end()) {
    //     // Snapshot trop ancien, on reset
    //     forceStateUpdate(serverState);
    //     return;
    // }

    // // Vérifie si l'état client diffère du serveur
    // if (statesDiffer(*it, serverState)) {
    //     // ROLLBACK nécessaire !
    //     performRollback(serverTick, serverState);
    // }
}

void TickSystem::saveSnapshot(EntityManager &entityManager)
{
    GameSnapshot snapshot;
    snapshot.tickNumber = currentTick;
    snapshot.timestamp = currentTick * TICK_DURATION;

    // Sauvegarde l'état de toutes les entités
    //  for (auto& entity : entityManager.getEntities()) {}

    snapshotHistory.push_back(snapshot);
}

bool TickSystem::statesDiffer(const GameSnapshot &clientSnapshot, const std::vector<EntitySnapshot> &serverState)
{
    // Compare l'état client vs serveur
    const float POSITION_TOLERANCE = 0.5f; // tolérance en pixels

    // for (const auto& serverEntity : serverState) {
    //     auto clientEntity = std::find_if(
    //         clientSnapshot.entities.begin(),
    //         clientSnapshot.entities.end(),
    //         [&](const EntitySnapshot& e) { return e.entityId == serverEntity.entityId; }
    //     );

    //     if (clientEntity == clientSnapshot.entities.end())
    //         return true; // Entité manquante

    //     // Vérifie la position
    //     float dx = std::abs(clientEntity->x - serverEntity.x);
    //     float dy = std::abs(clientEntity->y - serverEntity.y);

    //     if (dx > POSITION_TOLERANCE || dy > POSITION_TOLERANCE)
    //         return true; // Différence significative

    //     // Vérifie d'autres propriétés critiques
    //     if (clientEntity->health != serverEntity.health)
    //         return true;
    // }

    return false;
}

void TickSystem::performRollback(uint32_t serverTick, const std::vector<EntitySnapshot> &serverState)
{
    std::cout << "ROLLBACK from tick " << currentTick << " to " << serverTick << std::endl;

    // 1. Restaure l'état du serveur
    restoreState(serverState);

    // // 2. Rejoue tous les inputs depuis serverTick jusqu'à maintenant
    // auto inputIt = std::find_if(inputHistory.begin(), inputHistory.end(),
    //     [serverTick](const PlayerInput& inp) {
    //         return inp.tickNumber > serverTick;
    //     });

    // // Rejoue chaque input
    // EntityManager tempEntityManager; // Utilise ton vrai EntityManager
    // while (inputIt != inputHistory.end()) {
    //     applyInputs(tempEntityManager, *inputIt);
    //     simulateGameLogic(tempEntityManager);
    //     ++inputIt;
    // }

    currentTick = serverTick;
}

void TickSystem::restoreState(const std::vector<EntitySnapshot> &state)
{
    // Restaure l'état de toutes les entités
    // À adapter selon ton EntityManager
}

void TickSystem::simulateGameLogic(EntityManager &entityManager)
{
    // gameLogicSystem.update(entityManager, deltaTime);
    // backgroundSystem.update(entityManager, deltaTime);
    // movementSystem.update(entityManager, deltaTime);
    // playerSystem.update(entityManager, deltaTime);
    animationSystem.update(entityManager, TICK_DURATION);
    // inputSystem.update(entityManager, deltaTime);
    // boundarySyste>m.update(entityManager, deltaTime);
    // cleanupSystem.update(entityManager, deltaTime);
    // enemySystem.update(entityManager, deltaTime);
    // collisionSystem.update(entityManager);
    // laserWarningSystem.update(entityManager, deltaTime);
}

void TickSystem::forceStateUpdate(const std::vector<EntitySnapshot> &serverState)
{
    // Force la mise à jour de l'état (trop de décalage)
    restoreState(serverState);
    snapshotHistory.clear();
}

void TickSystem::cleanupHistory()
{
    // Garde seulement les MAX_SNAPSHOTS derniers snapshots
    while (snapshotHistory.size() > MAX_SNAPSHOTS)
    {
        snapshotHistory.pop_front();
    }
}
