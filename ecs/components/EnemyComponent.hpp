#pragma once
#include "../IComponent.hpp"
#include "systems/EnemySystem.hpp"

struct EnemyProperties
{
    float width;
    float height;
    float scaleX;
    float scaleY;
    float rotation;
    float top;
    float left;
    AnimatedSpriteComponent::SpritesheetLayout layout;
    int health;
    GraphicsManager::Texture texture;
    int numFrames;
    float interval;
};

static constexpr EnemyProperties BASIC_ENEMY = {32.8f,  32.3f,
                                                1.0f,   1.0f,
                                                -90.0f, 98.5f,
                                                32.3f,  AnimatedSpriteComponent::SpritesheetLayout::Horizontal,
                                                100,    GraphicsManager::Texture::ENEMY,
                                                1,      0.05f};

static constexpr EnemyProperties ROTATING_ENEMY = {16.0f,  23.0f,
                                                   2.0f,   2.0f,
                                                   -90.0f, 0.0f,
                                                   0.0f,   AnimatedSpriteComponent::SpritesheetLayout::Horizontal,
                                                   110,    GraphicsManager::Texture::ROTATING_ENEMY,
                                                   4,      0.15f};

static constexpr EnemyProperties PURPLE_ENEMY = {32.0f, 32.0f,
                                                 2.0f,  2.0f,
                                                 90.0f, 1.0f,
                                                 2.0f,  AnimatedSpriteComponent::SpritesheetLayout::Horizontal,
                                                 130,   GraphicsManager::Texture::PURPLE_ENEMY,
                                                 2,     0.15f};

static constexpr EnemyProperties BOSS = {110.0f, 112.0f,
                                         1.5f,   1.5f,
                                         0.0f,   0.0f,
                                         32.0f,  AnimatedSpriteComponent::SpritesheetLayout::Horizontal,
                                         300,    GraphicsManager::Texture::BOSS,
                                         1,      0.1f};

class EnemyComponent : public IComponent
{
  public:
    int type;
    int shootingType;
    float attackCooldown;
    float currentCooldown;
    int scoreValue;

    EnemyComponent(int t, float ac, int st, int scoreValue);

    void update(float deltaTime) override;
    std::vector<uint8_t> serialize() const override;
    static EnemyComponent deserialize(const uint8_t *data, size_t size);
};
