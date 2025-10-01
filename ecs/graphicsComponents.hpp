#pragma once
#include "components.hpp"
#include "components/Vector2D.hpp"
#include <SFML/Graphics.hpp>

struct AnimatedPlayerSpriteComponent : public Component
{
    sf::Sprite sprite;
    const sf::Texture *texture; // Use a pointer to the texture
    int currentFrame = 2;       // Default frame
    const int frameWidth;       // Width of each frame
    const int frameHeight;      // Height of each frame
    float animationInterval;    // Time between frame changes
    Vector2D scale = {1.0f, 1.0f};
    sf::Clock animationClock;
    enum Direction
    {
        Default,
        Up,
        Down
    } currentDirection = Default;

    AnimatedPlayerSpriteComponent(const sf::Texture &tex, int frameWidth, int frameHeight, float interval,
                            Vector2D scale = {1.0f, 1.0f})
        : texture(&tex), frameWidth(frameWidth), frameHeight(frameHeight), animationInterval(interval), scale(scale)
    {
        sprite.setTexture(*texture); // Use the texture pointer
        sprite.setScale(scale.x, scale.y);
        setFrame(currentFrame); // Set default frame
    }

    void setFrame(int frame)
    {
        sprite.setTextureRect(sf::IntRect(frameWidth * frame, 0, frameWidth, frameHeight));
    }

    void updateAnimation(Direction newDirection)
    {
        if (newDirection != currentDirection)
        {
            currentDirection = newDirection;
        }

        if (animationClock.getElapsedTime().asSeconds() >= animationInterval)
        {
            if (currentDirection == Up && currentFrame < 4)
            {
                currentFrame++;
            }
            else if (currentDirection == Down && currentFrame > 0)
            {
                currentFrame--;
            }
            else if (currentDirection == Default)
            {
                if (currentFrame > 2)
                    currentFrame--;
                else if (currentFrame < 2)
                    currentFrame++;
            }
            setFrame(currentFrame);
            animationClock.restart();
        }
    }

    // Override init method from Component
    void init() override
    {
    }
};
