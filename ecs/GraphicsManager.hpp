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

#pragma once

#include "../client/NetworkECSMediator.hpp"
#include "../ecs/components/AnimatedSpriteComponent.hpp"
#include "components/AnimatedSpriteComponent.hpp"
#include "textBox.hpp"
#include <SFML/Graphics.hpp>
#include <cstddef>
#include <memory>
#include <string>
#include <unordered_map>

#define TEXT_SIZE 24

class GraphicsManager
{
  private:
    sf::RenderWindow window;
    std::unordered_map<std::string, sf::Texture> textures;
    sf::Font font;
    std::unique_ptr<TextBox> _textbox;
    NetworkECSMediator _med;

  public:
    GraphicsManager(NetworkECSMediator med);
    ~GraphicsManager();

    bool init(const std::string &title, int width, int height, std::function<void(const char *)> startNetwork);
    void clear();
    void present();

    enum Texture
    {
        BACKGROUND,
        PLAYER,
        ENEMY,
        BULLET,
        EXPLOSION,
        BONUS_LIFE
    };

    sf::Texture &createColorTexture(int width, int height, sf::Uint8 r, sf::Uint8 g, sf::Uint8 b, sf::Uint8 a = 255);
    void storeTexture(const std::string &name, const sf::Texture &texture);
    sf::Texture *getTexture(const std::string &name);
    sf::Texture *getTexture(int tex);
    std::unique_ptr<TextBox> &getTextBox();
    sf::Font &getFont();

    void drawTexture(const sf::Texture &texture, float x, float y, float w, float h);
    void drawRect(float x, float y, float w, float h, sf::Uint8 r, sf::Uint8 g, sf::Uint8 b, sf::Uint8 a = 255);
    void drawText(const std::string &content, float x, float y, sf::Uint8 r = 255, sf::Uint8 g = 255,
                  sf::Uint8 b = 255);

    sf::RenderWindow &getWindow();
    sf::Texture &createTextureFromPath(const std::string &filePath, const std::string &name);
    bool registerTheTexture();
};

extern GraphicsManager *g_graphics;
