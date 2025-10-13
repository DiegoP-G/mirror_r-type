#pragma once
#include "../client/KeybindManager.hpp"
#include <SFML/Graphics.hpp>
#include <string>
#include <unordered_map>

class KeybindMenu
{
  public:
    KeybindMenu(KeybindManager &manager);

    void handleEvent(const sf::Event &event, const sf::RenderWindow &window);
    void draw(sf::RenderWindow &window);

    bool isOpen() const
    {
        return open;
    }

    bool isSoundMuted() const
    {
        return soundMuted;
    }

  private:
    KeybindManager &keybindManager;
    bool open = false;
    bool waitingForKey = false;
    std::string selectedAction;
    bool soundMuted = false;

    sf::RectangleShape button;
    sf::Font font;
    sf::Text buttonText;

    sf::RectangleShape menuBackground;
    std::unordered_map<std::string, sf::Text> keyLabels;
    std::unordered_map<std::string, sf::Keyboard::Key> defaultActions;

    sf::RectangleShape soundCheckbox;
    sf::Text soundLabel;
    sf::RectangleShape soundCheckMark;

    void openMenu();
    void closeMenu();
    void drawMenu(sf::RenderWindow &window);
    void rebindKey(sf::Keyboard::Key newKey);
    std::string keyToString(sf::Keyboard::Key key);
};
