#pragma once
#include "../components.hpp"
#include "Rectangle.hpp"
#include <SFML/Graphics.hpp>

class SpriteComponent : public Component
{
  public:
    sf::Texture *texture;
    Rectangle srcRect;
    int width, height;
    bool isVisible;
    uint8_t r, g, b, a;

    SpriteComponent();
    SpriteComponent(int w, int h, uint8_t red = 255, uint8_t green = 255, uint8_t blue = 255,
                    sf::Texture *texture = nullptr);

    void update(float deltaTime) override;
    void render() override;
    std::vector<uint8_t> serialize() const override;
    static SpriteComponent deserialize(const uint8_t *data);
};
