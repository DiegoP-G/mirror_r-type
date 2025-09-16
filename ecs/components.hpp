#pragma once
#include "ecs.hpp"
#include <SDL2/SDL.h>

// Vector2D for positions and movements
struct Vector2D {
    float x, y;
    
    Vector2D() : x(0.0f), y(0.0f) {}
    Vector2D(float x, float y) : x(x), y(y) {}
    
    Vector2D& operator+=(const Vector2D& v) {
        x += v.x;
        y += v.y;
        return *this;
    }
    
    Vector2D operator*(float scalar) const {
        return Vector2D(x * scalar, y * scalar);
    }
};

// Rectangle for collision and rendering
struct Rectangle {
    float x, y, w, h;
    
    Rectangle() : x(0), y(0), w(0), h(0) {}
    Rectangle(float x, float y, float w, float h) : x(x), y(y), w(w), h(h) {}
    
    bool intersects(const Rectangle& other) const {
        return !(x + w < other.x || other.x + other.w < x ||
                 y + h < other.y || other.y + other.h < y);
    }
};

// Component for position, scale, and rotation
struct TransformComponent : public Component {
    Vector2D position;
    Vector2D scale;
    float rotation;
    
    TransformComponent() : position(0, 0), scale(1, 1), rotation(0) {}
    TransformComponent(float x, float y) : position(x, y), scale(1, 1), rotation(0) {}
    TransformComponent(float x, float y, float sx, float sy, float r) : 
        position(x, y), scale(sx, sy), rotation(r) {}
        
    void update(float deltaTime) override {}
};

// Component for movement
struct VelocityComponent : public Component {
    Vector2D velocity;
    float maxSpeed;
    
    VelocityComponent() : velocity(0, 0), maxSpeed(5.0f) {}
    VelocityComponent(float x, float y) : velocity(x, y), maxSpeed(5.0f) {}
    VelocityComponent(float x, float y, float max) : velocity(x, y), maxSpeed(max) {}
};

// Component for rendering with SDL
struct SpriteComponent : public Component {
    SDL_Texture* texture;
    Rectangle srcRect;  // Source rectangle in texture
    int width, height;  // Render size
    bool isVisible;
    Uint8 r, g, b, a;   // Color modulation
    
    SpriteComponent() : texture(nullptr), isVisible(true), width(32), height(32), 
                       r(255), g(255), b(255), a(255) {}
    
    SpriteComponent(int w, int h, Uint8 red = 255, Uint8 green = 255, Uint8 blue = 255) 
        : texture(nullptr), isVisible(true), width(w), height(h), r(red), g(green), b(blue), a(255) {}
    
    void render() override {
        // Rendering is handled by RenderSystem
    }
};

// Component for collision
struct ColliderComponent : public Component {
    Rectangle hitbox;
    bool isTrigger;
    
    ColliderComponent() : isTrigger(false) {}
    ColliderComponent(float w, float h) : hitbox(0, 0, w, h), isTrigger(false) {}
};

// Component for player/enemy health
struct HealthComponent : public Component {
    float health;
    float maxHealth;
    
    HealthComponent(float h) : health(h), maxHealth(h) {}
};

// Component for player input
struct InputComponent : public Component {
    bool up, down, left, right, fire;
    
    InputComponent() : up(false), down(false), left(false), right(false), fire(false) {}
};

// Component for projectiles
struct ProjectileComponent : public Component {
    float damage;
    float lifeTime;
    float remainingLife;
    EntityID owner;
    
    ProjectileComponent(float d, float lt, EntityID o) : 
        damage(d), lifeTime(lt), remainingLife(lt), owner(o) {}
        
    void update(float deltaTime) override {
        remainingLife -= deltaTime;
    }
};

// Component for enemies
struct EnemyComponent : public Component {
    int type;
    float attackCooldown;
    float currentCooldown;
    
    EnemyComponent(int t, float ac) : type(t), attackCooldown(ac), currentCooldown(0) {}
    
    void update(float deltaTime) override {
        if (currentCooldown > 0) {
            currentCooldown -= deltaTime;
        }
    }
};

// Flappy Bird specific components
struct BirdComponent : public Component {
    float jumpStrength;
    bool isDead;
    
    BirdComponent(float jump = -400.0f) : jumpStrength(jump), isDead(false) {}
};

struct PipeComponent : public Component {
    bool isTopPipe;
    float gapHeight;
    bool hasScored;
    
    PipeComponent(bool top = false, float gap = 150.0f) : isTopPipe(top), gapHeight(gap), hasScored(false) {}
};

struct GravityComponent : public Component {
    float gravity;
    float terminalVelocity;
    
    GravityComponent(float g = 800.0f, float tv = 600.0f) : gravity(g), terminalVelocity(tv) {}
};

// Generic jump component instead of BirdComponent
struct JumpComponent : public Component {
    float jumpStrength;
    bool canJump;
    
    JumpComponent(float jump = -400.0f) : jumpStrength(jump), canJump(true) {}
};

struct GameStateComponent : public Component {
    int score;
    bool gameOver;
    bool started;
    
    GameStateComponent() : score(0), gameOver(false), started(false) {}
};