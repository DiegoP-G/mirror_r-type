#pragma once
#include "ecs.hpp"
#include "components.hpp"
#include <cstdint>

// Network Components
struct NetworkComponent : public Component {
    uint32_t networkID;
    bool isNetworked;
    bool isDirty = false; // Needs to be synchronized
    
    NetworkComponent(uint32_t id = 0, bool networked = true) 
        : networkID(id), isNetworked(networked) {}
};

struct ServerAuthorityComponent : public Component {
    bool serverControlled = true;
    float lastSyncTime = 0.0f;
    
    ServerAuthorityComponent(bool controlled = true) : serverControlled(controlled) {}
};

struct ClientPredictionComponent : public Component {
    Vector2D predictedPosition;
    Vector2D serverPosition;
    float reconciliationThreshold = 5.0f;
    
    ClientPredictionComponent() = default;
};

struct PlayerComponent : public Component {
    uint32_t playerID;
    bool isLocalPlayer = false;
    
    PlayerComponent(uint32_t id, bool local = false) : playerID(id), isLocalPlayer(local) {}
};