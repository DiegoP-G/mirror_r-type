#pragma once
#include "../IComponent.hpp"
#include "Rect.hpp"
#include <SFML/Graphics.hpp>

class SpriteComponent : public IComponent
{
public:
    int spriteTexture;
    Rect srcRect;
    int width, height;
    bool isVisible;
    uint8_t r, g, b, a;

    SpriteComponent();
    SpriteComponent(int w, int h, uint8_t red = 255, uint8_t green = 255, uint8_t blue = 255, int texture = 0);

    void update(float deltaTime) override;
    void render() override;
    std::vector<uint8_t> serialize() const override;
    static SpriteComponent deserialize(const uint8_t* data, size_t size);
};