#pragma once
#include <functional>
#include <SFML/Graphics.hpp>
#include <string>

class TextBox {
private:
    sf::RectangleShape box;
    sf::Text text;
    sf::Font font;
    bool isFocused;
    bool display = true;
    std::string input;
    std::string value;
    std::function<void(const char *)> _startNetwork;

public:
    TextBox(sf::Font &font, std::function<void(const char *)>, float width = 300.f, float height = 40.f);

    void setPosition(float x, float y);
    void checkInFocus(sf::Vector2i mousePos);
    void typeInBox(sf::Event event);
    void draw(sf::RenderWindow& window);
    std::string getText() const;
    bool getDisplayValue() const;
};
