#include "textBox.hpp"
#include <SFML/Graphics/Font.hpp>
#include <iostream>
#include <stdexcept>

TextBox::TextBox(sf::Font &font, std::function<void(const char *)> startNetwork, float width, float height)
    : isFocused(false), input(""), value(""), font(font), _startNetwork(startNetwork)
{
    box.setSize({width, height});
    box.setFillColor(sf::Color::White);
    box.setOutlineThickness(2);
    box.setOutlineColor(sf::Color::Black);

    text.setFont(font);
    text.setCharacterSize(20);
    text.setFillColor(sf::Color::Black);
}

void TextBox::setPosition(float x, float y)
{
    box.setPosition(x, y);
    // text.setPosition(x, y);
    text.setPosition(x + 10.f, y + 5.f);
}

void TextBox::checkInFocus(sf::Vector2i mousePos)
{
    if (box.getGlobalBounds().contains(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y)))
        isFocused = true;
    else
        isFocused = false;

    box.setOutlineColor(isFocused ? sf::Color::Blue : sf::Color::Black);
}

void TextBox::typeInBox(sf::Event event)
{
    if (!isFocused)
        return;

    if (event.type == sf::Event::TextEntered)
    {
        // backspace
        if (event.text.unicode == 8 && !input.empty())
        {
            input.pop_back();
            // enter
        }
        else if (event.text.unicode == 13)
        {
            value = input;
            isFocused = false;
            box.setOutlineColor(sf::Color::Black);
            _startNetwork(input.c_str());
            display = false;
        }
        else if (event.text.unicode >= 32 && event.text.unicode < 128)
        {
            input += static_cast<char>(event.text.unicode);
        }

        text.setString(input);
    }
}

void TextBox::draw(sf::RenderWindow &window)
{
    window.draw(box);
    std::cout << std::string(text.getString()) << std::endl;
    window.draw(text);
}

std::string TextBox::getText() const
{
    return value;
}

bool TextBox::getDisplayValue() const
{
    return display;
}

sf::Vector2f TextBox::getSize() const
{
    return box.getSize();
}

void TextBox::setAtCenter(sf::RenderWindow &window)
{
    float windowWidth = window.getSize().x;
    float windowHeight = window.getSize().y;

    float textBoxWidth = getSize().x;
    float textBoxHeight = getSize().y;

    float posX = (windowWidth - textBoxWidth) / 2.0f;
    float posY = (windowHeight - textBoxHeight) / 2.0f;

    setPosition(posX, posY);
}