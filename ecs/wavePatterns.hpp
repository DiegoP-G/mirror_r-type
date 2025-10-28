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
#include <ctime>


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

inline std::vector<Vector2D> randomPattern(int count, float cx, float cy)
{
    std::srand(std::time(nullptr));

    std::vector<Vector2D> positions;
    float spacing = 40.0f;
    for (int i = 0; i < count; i++)
    {
        int x = rand() % 50 + windowWidth;
        int y = (rand() % (windowHeight - 100)) + 50;
        positions.push_back(Vector2D(x, y));
    }
    return positions;
}