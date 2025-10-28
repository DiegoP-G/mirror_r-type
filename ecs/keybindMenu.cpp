#include "keybindMenu.hpp"
#include "../client/KeybindManager.hpp"
#include "GraphicsManager.hpp"
#include <iostream>

KeybindMenu::KeybindMenu(KeybindManager &manager) : keybindManager(manager)
{
    font = g_graphics->getFont();

    button.setSize({100, 40});
    button.setFillColor(sf::Color(100, 100, 255));
    button.setPosition(50, 150);

    buttonText.setFont(font);
    buttonText.setString("Keybinds");
    buttonText.setCharacterSize(18);
    buttonText.setFillColor(sf::Color::White);
    buttonText.setPosition(button.getPosition().x + 5, button.getPosition().y + 5);

    menuBackground.setSize({300, 300}); // Augmenté pour inclure la checkbox
    menuBackground.setFillColor(sf::Color(50, 50, 50, 230));
    menuBackground.setPosition(200, 100);

    defaultActions = {{"Up", sf::Keyboard::Up},       {"Down", sf::Keyboard::Down},  {"Left", sf::Keyboard::Left},
                      {"Right", sf::Keyboard::Right}, {"Fire", sf::Keyboard::Space}, {"Warp", sf::Keyboard::W}};

    float yOffset = 120;
    for (auto &[action, key] : defaultActions)
    {
        sf::Text text;
        text.setFont(font);
        text.setCharacterSize(20);
        text.setFillColor(sf::Color::White);
        text.setString(action + " : " + keyToString(key));
        text.setPosition(220, yOffset);
        keyLabels[action] = text;
        yOffset += 40;
    }

    // Configuration de la checkbox pour le son
    soundCheckbox.setSize({25, 25});
    soundCheckbox.setFillColor(sf::Color(200, 200, 200));
    soundCheckbox.setOutlineColor(sf::Color::White);
    soundCheckbox.setOutlineThickness(2);
    soundCheckbox.setPosition(220, yOffset + 20);

    soundCheckMark.setSize({15, 15});
    soundCheckMark.setFillColor(sf::Color(0, 255, 0));
    soundCheckMark.setPosition(225, yOffset + 25);

    soundLabel.setFont(font);
    soundLabel.setCharacterSize(20);
    soundLabel.setFillColor(sf::Color::White);
    soundLabel.setString("Mute Sound");
    soundLabel.setPosition(260, yOffset + 20);
}

void KeybindMenu::handleEvent(const sf::Event &event, const sf::RenderWindow &window)
{
    if (event.type == sf::Event::MouseButtonPressed)
    {
        sf::Vector2f mousePos = (sf::Vector2f)sf::Mouse::getPosition(window);

        if (!open && button.getGlobalBounds().contains(mousePos))
        {
            openMenu();
            return;
        }

        if (open)
        {
            if (!menuBackground.getGlobalBounds().contains(mousePos))
            {
                closeMenu();
                return;
            }

            // Vérifier si l'utilisateur a cliqué sur la checkbox du son
            if (soundCheckbox.getGlobalBounds().contains(mousePos))
            {
                soundMuted = !soundMuted;
                if (soundMuted)
                {
                    g_graphics->stopSound("music");
                }
                else
                {
                    g_graphics->playSound("music", true);
                }
                std::cout << "[Sound] " << (soundMuted ? "Muted" : "Unmuted") << "\n";
                return;
            }

            // Vérifier si l'utilisateur a cliqué sur une action
            for (auto &[action, text] : keyLabels)
            {
                if (text.getGlobalBounds().contains(mousePos))
                {
                    waitingForKey = true;
                    selectedAction = action;
                    text.setFillColor(sf::Color::Yellow);
                    return;
                }
            }
        }
    }

    if (waitingForKey && event.type == sf::Event::KeyPressed)
    {
        rebindKey(event.key.code);
        waitingForKey = false;
        selectedAction.clear();
    }
}

void KeybindMenu::draw(sf::RenderWindow &window)
{
    // Toujours afficher le bouton
    window.draw(button);
    window.draw(buttonText);

    if (open)
        drawMenu(window);
}

void KeybindMenu::openMenu()
{
    open = true;
}

void KeybindMenu::closeMenu()
{
    open = false;
    waitingForKey = false;
    selectedAction.clear();

    // Réinitialiser les couleurs
    for (auto &[_, text] : keyLabels)
        text.setFillColor(sf::Color::White);
}

void KeybindMenu::drawMenu(sf::RenderWindow &window)
{
    window.draw(menuBackground);
    for (auto &[_, text] : keyLabels)
        window.draw(text);

    // Dessiner la checkbox et le label du son
    window.draw(soundCheckbox);
    if (!soundMuted)
    {
        window.draw(soundCheckMark);
    }
    window.draw(soundLabel);
}

void KeybindMenu::rebindKey(sf::Keyboard::Key newKey)
{
    if (selectedAction.empty())
        return;

    auto &text = keyLabels[selectedAction];
    text.setFillColor(sf::Color::White);
    text.setString(selectedAction + " : " + keyToString(newKey));

    sf::Keyboard::Key oldKey = defaultActions[selectedAction];

    keybindManager.setKeybind(newKey, oldKey);

    defaultActions[selectedAction] = newKey;

    std::cout << "[Keybind] " << selectedAction << " bound to " << keyToString(newKey) << "\n";
}

std::string KeybindMenu::keyToString(sf::Keyboard::Key key)
{
    static const std::unordered_map<sf::Keyboard::Key, std::string> keyNames = {{sf::Keyboard::Unknown, "Unknown"},
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
                                                                                {sf::Keyboard::Down, "Down"}};

    auto it = keyNames.find(key);
    if (it != keyNames.end())
        return it->second;

    return "Unknown";
}