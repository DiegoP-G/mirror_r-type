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

#include "GraphicsManager.hpp"
#include "../client/NetworkECSMediator.hpp"
#include "textBox.hpp"
#include "../client/assetsPath.hpp"
#include <SFML/Graphics/Font.hpp>
#include <iostream>
#include <memory>

GraphicsManager *g_graphics = nullptr;

GraphicsManager::GraphicsManager(NetworkECSMediator med) : _med(med)
{
}

bool GraphicsManager::registerTheTexture()
{
    createTextureFromPath(PathFormater::formatAssetPath("assets/sprites/background.jpg"), "background");
    createTextureFromPath(PathFormater::formatAssetPath("assets/sprites/playerSpritesheet.png"), "player");
    createTextureFromPath(PathFormater::formatAssetPath("assets/sprites/ennemy.png"), "enemy");
    createTextureFromPath(PathFormater::formatAssetPath("assets/sprites/ennemy.png"), "bullet");
    createTextureFromPath(PathFormater::formatAssetPath("assets/sprites/explosion.png"), "explosion");

    return true;
}

GraphicsManager::~GraphicsManager()
{
}

bool GraphicsManager::init(const std::string &title, int width, int height, std::function<void(const char *)> startNetwork)
{
    window.create(sf::VideoMode(width, height), title, sf::Style::Close);
    registerTheTexture();
    if (!window.isOpen())
    {
        std::cerr << "Error: SFML Window could not be created" << std::endl;
        return false;
    }

    if (!font.loadFromFile(PathFormater::formatAssetPath(fontPath)))
    {
        std::cerr << "Warning: SFML Failed to load font, using default font" << std::endl;
    }
    _textbox = std::make_unique<TextBox>(font, startNetwork, 400, 50);
    std::cout << "SFML initialized successfully" << std::endl;
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

sf::Texture &GraphicsManager::createColorTexture(int width, int height, sf::Uint8 r, sf::Uint8 g, sf::Uint8 b,
                                                 sf::Uint8 a)
{
    sf::Image img;
    img.create(width, height, sf::Color(r, g, b, a));

    sf::Texture tex;
    if (!tex.loadFromImage(img))
    {
        std::cerr << "Error: SFML Failed to created texture" << std::endl;
    }
    std::string key = "color_" + std::to_string(textures.size());
    textures[key] = tex;
    return textures[key];
}

void GraphicsManager::storeTexture(const std::string &name, const sf::Texture &texture)
{
    textures[name] = texture;
}

sf::Texture *GraphicsManager::getTexture(const std::string &name)
{
    auto it = textures.find(name);
    if (it != textures.end())
        return &it->second;
    return nullptr;
}

sf::Texture *GraphicsManager::getTexture(int tex)
{
    switch (tex)
    {
    case BACKGROUND:
        return getTexture("background");
    case PLAYER:
        return getTexture("player");
    case ENEMY:
        return getTexture("basic_enemy");
    case BULLET:
        return getTexture("bullet");
    case EXPLOSION:
        return getTexture("explosion");
    default:
        return nullptr;
    }
}

void GraphicsManager::drawTexture(const sf::Texture &texture, float x, float y, float w, float h)
{
    sf::Sprite sprite(texture);
    sprite.setPosition(x, y);
    sprite.setScale(w / texture.getSize().x, h / texture.getSize().y);

    window.draw(sprite);
}

void GraphicsManager::drawRect(float x, float y, float w, float h, sf::Uint8 r, sf::Uint8 g, sf::Uint8 b, sf::Uint8 a)
{
    sf::RectangleShape rect(sf::Vector2f((float)w, (float)h));
    rect.setPosition((float)x, (float)y);
    rect.setFillColor(sf::Color(r, g, b, a));

    window.draw(rect);
}

void GraphicsManager::drawText(const std::string &content, float x, float y, sf::Uint8 r, sf::Uint8 g, sf::Uint8 b)
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

std::unique_ptr<TextBox>& GraphicsManager::getTextBox()
{
    return _textbox;
}

sf::Texture &GraphicsManager::createTextureFromPath(const std::string &filePath, const std::string &name)
{
    sf::Texture tex;
    if (!tex.loadFromFile(filePath))
    {
        std::cerr << "Error: SFML Failed to load texture from file: " << filePath << std::endl;
        return createColorTexture(32, 32, 255, 255, 0); // Yellow player;
    }

    textures[name] = tex;
    return textures[name];
}

sf::Font &GraphicsManager::getFont()
{
    return font;
}