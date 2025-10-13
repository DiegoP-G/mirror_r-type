#pragma once
#include <SFML/Window/Keyboard.hpp>
#include <unordered_map>

class KeybindManager
{
  public:
    KeybindManager();

    void setKeybind(sf::Keyboard::Key input, sf::Keyboard::Key mapped);
    sf::Keyboard::Key getKeybind(sf::Keyboard::Key input) const;
    sf::Keyboard::Key processKeyInput(sf::Keyboard::Key input) const;
    void printKeybinds() const;

  private:
    std::unordered_map<sf::Keyboard::Key, sf::Keyboard::Key> keybindMap;
};
