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

#ifndef GRAPHICS_HPP
#define GRAPHICS_HPP

#include "../ecs/components.hpp"
#include <SFML/Graphics.hpp>
#include <string>
#include <unordered_map>

#define TEXT_SIZE 24

class GraphicsManager
{
  private:
    sf::RenderWindow window;
    std::unordered_map<std::string, sf::Texture> textures;
    sf::Font font;

  public:
    GraphicsManager();
    ~GraphicsManager();

    bool init(const std::string &title, int width, int height);
    void clear();
    void present();

    sf::Texture &createColorTexture(int width, int height, sf::Uint8 r, sf::Uint8 g, sf::Uint8 b, sf::Uint8 a = 255);
    void storeTexture(const std::string &name, const sf::Texture &texture);
    sf::Texture *getTexture(const std::string &name);

    void drawTexture(const sf::Texture &texture, float x, float y, float w, float h);
    void drawRect(float x, float y, float w, float h, sf::Uint8 r, sf::Uint8 g, sf::Uint8 b, sf::Uint8 a = 255);
    void drawText(const std::string &content, float x, float y, sf::Uint8 r = 255, sf::Uint8 g = 255,
                  sf::Uint8 b = 255);

    sf::RenderWindow &getWindow();
    void drawAnimatedSprite(AnimatedSpriteComponent &animatedSprite, float, float);
    sf::Texture &createTextureFromPath(const std::string &filePath, const std::string &name);
};

extern GraphicsManager *g_graphics;

#endif // GRAPHICS_HPP
