#pragma once
#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>

#include <windows.h>
#endif
#include <SFML/Graphics.hpp>
#include <functional>
#include <string>

class TextBox
{
  private:
    sf::RectangleShape box;
    sf::Text text;
    sf::Font font;
    bool isFocused;
    bool display = true;
    bool isPasswordMode = false; // Add this line
    std::string input;
    std::string value;
    std::function<void(const char *)> _startNetwork;

  public:
    TextBox(sf::Font &font, std::function<void(const char *)>, float width = 300.f, float height = 40.f);

    void setPosition(float x, float y);
    void setAtCenter(sf::RenderWindow &window);
    void checkInFocus(sf::Vector2i mousePos);
    void typeInBox(sf::Event event);
    void draw(sf::RenderWindow &window);
    void setPasswordMode(bool enable); // Add this line
    std::string getText() const;
    bool getDisplayValue() const;
    sf::Vector2f getSize() const;
};
