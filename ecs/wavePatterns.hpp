#pragma once
#include "../client/windowSize.hpp"
#include "GraphicsManager.hpp"
#include "allComponentsInclude.hpp"
#include "components/Vector2D.hpp"
#include "enemyFactory.hpp"
#include "entityManager.hpp"
#include <functional>
#include <math.h>
#include <vector>

// using PatternFunc = std::function<std::vector<Vector2D>(int, float, float)>;

inline std::vector<Vector2D> backslashPattern(int count, float cx, float cy)
{
    std::vector<Vector2D> positions;
    if (count < 1)
        return positions;
    float spacing = 40.0f;
    int mid = count / 2;
    for (int i = 0; i < count; ++i)
    {
        int dx = std::abs(i - mid);
        float x = cx + dx * spacing;
        float y = cy + (i - mid) * spacing;
        if (i < mid)
            x = cx - dx * spacing;
        positions.push_back(Vector2D(x, y));
    }
    return positions;
}

inline std::vector<Vector2D> diamondPattern(int count, float cx, float cy)
{
    std::vector<Vector2D> positions;
    if (count < 1)
        return positions;

    float spacing = 40.0f;  // Distance between enemies
    int layers = count / 4; // Number of layers (top to bottom)

    int placed = 0;

    // Top half (including center row)
    for (int i = 0; i <= layers && placed < count; ++i)
    {
        float y = cy - i * spacing; // Move up for each layer
        if (i == 0)
        {
            positions.push_back(Vector2D(cx, y)); // Top center
            placed++;
        }
        else
        {
            if (placed < count)
                positions.push_back(Vector2D(cx - i * spacing, y)); // Left
            placed++;
            if (placed < count)
                positions.push_back(Vector2D(cx + i * spacing, y)); // Right
            placed++;
        }
    }

    // Bottom half
    for (int i = layers - 1; i >= 0 && placed < count; --i)
    {
        float y = cy + (layers - i) * spacing;
        if (i == 0)
        {
            positions.push_back(Vector2D(cx, y - (120 * (i + 1)))); // Bottom center
            placed++;
        }
        else
        {
            if (placed < count)
                positions.push_back(Vector2D(cx - i * spacing, y - (120 * (i + 1)))); // Left
            placed++;
            if (placed < count)
                positions.push_back(Vector2D(cx + i * spacing, y - (120 * (i + 1)))); // Right
            placed++;
        }
    }

    return positions;
}

inline std::vector<Vector2D> linePattern(int count, float cx, float cy)
{
    std::vector<Vector2D> positions;
    float spacing = 40.0f;
    for (int i = 0; i < count; ++i)
    {
        positions.push_back(Vector2D(cx, cy + (i - count / 2) * spacing));
    }
    return positions;
}

inline std::vector<Vector2D> vPattern(int count, float cx, float cy)
{
    std::vector<Vector2D> positions;
    float spacing = 40.0f;
    for (int i = 0; i < count; ++i)
    {
        float x = cx + std::abs(i - count / 2) * spacing;
        float y = cy + (i - count / 2) * spacing;
        positions.push_back(Vector2D(x, y));
    }
    return positions;
}

// class WaveSystem
// {
//   private:
//     struct Wave
//     {
//         float spawnDelay; // Time after previous wave
//         int enemyCount;
//         std::string enemyType;
//         PatternFunc pattern;
//     };

//     // Default waves
//     std::vector<Wave> waves {
//         {0.0f, 4, "basic_enemy", linePattern},
//         {2.5f, 8, "basic_enemy", diamondPattern},
//         {2.5f, 7, "basic_enemy", vPattern},
//         {2.5f, 10, "basic_enemy", backslashPattern},
//     };

//     float waveTimer = 0.0f;
//     bool waveActive = false;
//     size_t currentWave = 0;

//   public:

//     void update(EntityManager &entityManager, float deltaTime)
//     {
//         if (currentWave >= waves.size())
//             return; // All waves done

//         waveTimer += deltaTime;

//         // Wait for spawnDelay before starting next wave
//         if (!waveActive && waveTimer >= waves[currentWave].spawnDelay)
//         {
//             std::cout << "Spawning wave " << currentWave + 1 << std::endl;
//             spawnWave(entityManager, waves[currentWave]);
//             waveActive = true;
//         }

//         // Check if all enemies from this wave are destroyed
//         if (waveActive && entityManager.getEntitiesWithComponents<EnemyComponent>().empty())
//         {
//             // Prepare for next wave
//             waveActive = false;
//             waveTimer = 0.0f;
//             currentWave++;
//         }
//     }

//   private:
//     void spawnWave(EntityManager &entityManager, const Wave &wave)
//     {
//         float cx = windowWidth + 50.0f;
//         float cy = windowHeight / 2.0f;
//         std::vector<Vector2D> positions = wave.pattern(wave.enemyCount, cx, cy);

//         for (const auto &pos : positions)
//         {
//             EnemyFactory::createEnemy(entityManager, wave.enemyType, pos);
//         }
//     }
// };