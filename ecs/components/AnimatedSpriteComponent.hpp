#pragma once
#include "../components.hpp"
#include "Vector2D.hpp"
#include <SFML/Graphics.hpp>

class AnimatedSpriteComponent : public Component {
public:
    AnimatedSpriteComponent(const sf::Texture& texture, int left, int top, int frameWidth, int frameHeight, int totalFrames, float interval)
        : sprite(texture), frameHeight(frameHeight), frameWidth(frameWidth), top(top), left(left), totalFrames(totalFrames), interval(interval), currentFrame(0), elapsedTime(0.0f) {
        // Calculate frame size based on texture size and total frames
        sprite.setTextureRect(sf::IntRect(left, top, frameWidth, frameHeight));
    }

    void update(float deltaTime) {
        if (totalFrames <= 1) return;
    
        elapsedTime += deltaTime;

        if (elapsedTime >= interval) {
            elapsedTime -= interval;
            currentFrame++;

            if (currentFrame >= totalFrames) {
                currentFrame = 0;
            }

            // Update the texture rectangle to display the correct frame
            sprite.setTextureRect(sf::IntRect(currentFrame * frameWidth, top, frameWidth, frameHeight));
        }
    }

    sf::Sprite sprite;
    int frameWidth;
    int frameHeight;
    int top;
    int left;
    int totalFrames;
    int currentFrame;
    float interval;
    float elapsedTime;
};