#include "KeybindManager.hpp"
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
#include <SFML/Window/Keyboard.hpp>
#include <iostream>

KeybindManager::KeybindManager()
{
    keybindMap = {{sf::Keyboard::Up, sf::Keyboard::Up},       {sf::Keyboard::Down, sf::Keyboard::Down},
                  {sf::Keyboard::Left, sf::Keyboard::Left},   {sf::Keyboard::Right, sf::Keyboard::Right},
                  {sf::Keyboard::Space, sf::Keyboard::Space}, {sf::Keyboard::W, sf::Keyboard::W}};
}

void KeybindManager::setKeybind(sf::Keyboard::Key input, sf::Keyboard::Key mapped)
{
    keybindMap[input] = mapped;
}

sf::Keyboard::Key KeybindManager::getKeybind(sf::Keyboard::Key input) const
{
    auto it = keybindMap.find(input);
    if (it != keybindMap.end())
        return it->second;
    return sf::Keyboard::Unknown;
}

sf::Keyboard::Key KeybindManager::processKeyInput(sf::Keyboard::Key input) const
{
    auto it = keybindMap.find(input);
    if (it != keybindMap.end())
        return it->second;

    for (const auto &pair : keybindMap)
    {
        if (pair.second == input)
            return sf::Keyboard::Unknown;
    }

    return input;
}

std::string keyToString(sf::Keyboard::Key key)
{
    static const std::unordered_map<sf::Keyboard::Key, std::string> keyNames = {
        {sf::Keyboard::Unknown, "Unknown"},
        {sf::Keyboard::A, "A"},
        {sf::Keyboard::B, "B"},
        {sf::Keyboard::C, "C"},
        {sf::Keyboard::D, "D"},
        {sf::Keyboard::E, "E"},
        {sf::Keyboard::F, "F"},
        {sf::Keyboard::G, "G"},
        {sf::Keyboard::H, "H"},
        {sf::Keyboard::I, "I"},
        {sf::Keyboard::J, "J"},
        {sf::Keyboard::K, "K"},
        {sf::Keyboard::L, "L"},
        {sf::Keyboard::M, "M"},
        {sf::Keyboard::N, "N"},
        {sf::Keyboard::O, "O"},
        {sf::Keyboard::P, "P"},
        {sf::Keyboard::Q, "Q"},
        {sf::Keyboard::R, "R"},
        {sf::Keyboard::S, "S"},
        {sf::Keyboard::T, "T"},
        {sf::Keyboard::U, "U"},
        {sf::Keyboard::V, "V"},
        {sf::Keyboard::W, "W"},
        {sf::Keyboard::X, "X"},
        {sf::Keyboard::Y, "Y"},
        {sf::Keyboard::Z, "Z"},
        {sf::Keyboard::Num0, "0"},
        {sf::Keyboard::Num1, "1"},
        {sf::Keyboard::Num2, "2"},
        {sf::Keyboard::Num3, "3"},
        {sf::Keyboard::Num4, "4"},
        {sf::Keyboard::Num5, "5"},
        {sf::Keyboard::Num6, "6"},
        {sf::Keyboard::Num7, "7"},
        {sf::Keyboard::Num8, "8"},
        {sf::Keyboard::Num9, "9"},
        {sf::Keyboard::Escape, "Escape"},
        {sf::Keyboard::LControl, "LControl"},
        {sf::Keyboard::LShift, "LShift"},
        {sf::Keyboard::LAlt, "LAlt"},
        {sf::Keyboard::Space, "Space"},
        {sf::Keyboard::Enter, "Enter"},
        {sf::Keyboard::Backspace, "Backspace"},
        {sf::Keyboard::Tab, "Tab"},
        {sf::Keyboard::Left, "Left"},
        {sf::Keyboard::Right, "Right"},
        {sf::Keyboard::Up, "Up"},
        {sf::Keyboard::Down, "Down"} // add more keys if needed
    };

    auto it = keyNames.find(key);
    return (it != keyNames.end()) ? it->second : "Unknown";
}

void KeybindManager::printKeybinds() const
{
    std::cout << "--- Current Keybinds ---\n";
    for (const auto &[input, mapped] : keybindMap)
    {
        std::cout << keyToString(input) << " -> " << keyToString(mapped) << "\n";
    }
    std::cout << "------------------------\n";
}