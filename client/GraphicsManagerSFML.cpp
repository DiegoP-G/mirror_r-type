/*
* < Kawabonga ! >
* ------------------------------
*    \   ^__^
*     \  (oo)\_______
*        (__)\       )\/\
*            ||----w |
*            ||     ||
* 
*/

#include "GraphicsManagerSFML.hpp"
#include <iostream>

GraphicsManager *g_graphics = nullptr;

GraphicsManager::GraphicsManager() {}

GraphicsManager::~GraphicsManager() {}

bool GraphicsManager::init(const std::string &title, int width, int height)
{
    window.create(sf::VideoMode(width, height), title, sf::Style::Close);

    if (!window.isOpen()) {
        std::cerr << "Error: SFML Window could not be created" << std::endl;
        return false;
    }

    if (font.loadFromFile("/assets/fonts/upheatvtt.ttf")) {
        std::cerr << "Error: SFML Failed to load font" << std::endl;
        return false;
    }
    return true;
}

void GraphicsManager::clear()
{
    window.clear(sf::Color::Black);
}

void GraphicsManager::present()
{
    window.display();
}

sf::Texture &GraphicsManager::createColorTexture(int width, int height,
    sf::Uint8 r, sf::Uint8 g, sf::Uint8 b, sf::Uint8 a)
{
    sf::Image img;
    img.create(width, height, sf::Color(r, g, b, a));

    sf::Texture tex;
    if (!tex.loadFromImage(img)) {
        std::cerr << "Error: SFML Failed to created texture" << std::endl;
    }
    std::string key = "color_" + std::to_string(textures.size());
    textures[key] = tex;
    return textures[key];
}

void GraphicsManager::storeTexture(const std::string &name,
    const sf::Texture &texture)
{
    textures[name] = texture;
}

sf::Texture *GraphicsManager::getTexture(const std::string &name) {
    auto it = textures.find(name);
    if (it != textures.end())
        return &it->second;
    return nullptr;
}

void GraphicsManager::drawTexture(const sf::Texture &texture, int x, int y,
    int w, int h)
{
    sf::Sprite sprite(texture);
    sprite.setPosition((float)x, (float)y);
    sprite.setScale(
        (float)w / texture.getSize().x,
        (float)h / texture.getSize().y
    );

    window.draw(sprite);
}

void GraphicsManager::drawRect(int x, int y, int w, int h, sf::Uint8 r,
    sf::Uint8 g, sf::Uint8 b, sf::Uint8 a)
{
    sf::RectangleShape rect(sf::Vector2f((float)w, (float)h));
    rect.setPosition((float)x, (float)y);
    rect.setFillColor(sf::Color(r, g, b, a));

    window.draw(rect);
}

void GraphicsManager::drawText(const std::string& content, int x, int y,
    sf::Uint8 r, sf::Uint8 g, sf::Uint8 b)
{
    sf::Text text;
    text.setFont(font);
    text.setString(content);
    text.setCharacterSize(TEXT_SIZE);
    text.setFillColor(sf::Color(r, g, b));
    text.setPosition((float)x, (float)y);
    
    window.draw(text);
}

sf::RenderWindow &GraphicsManager::getWindow()
{
    return window;
}
