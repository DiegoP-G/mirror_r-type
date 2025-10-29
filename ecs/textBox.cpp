#include "textBox.hpp"
#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#include <windows.h>
#else
#include <arpa/inet.h>
#include <netdb.h>
#endif
#include <SFML/Graphics/Font.hpp>
#include <cstring>
#include <iostream>
#include <stdexcept>

std::string getIpFromString(const std::string &hostname)
{
    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    int status = getaddrinfo(hostname.c_str(), nullptr, &hints, &res);
    if (status != 0)
    {
        std::cerr << "getaddrinfo error: " << gai_strerror(status) << std::endl;
        return "";
    }

    struct sockaddr_in *ipv4 = (struct sockaddr_in *)res->ai_addr;
    char ipstr[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(ipv4->sin_addr), ipstr, INET_ADDRSTRLEN);

    freeaddrinfo(res);
    return std::string(ipstr);
}

TextBox::TextBox(sf::Font &font, std::function<void(const char *)> startNetwork, float width, float height)
    : isFocused(false), input(""), value(""), font(font), _startNetwork(startNetwork), isPasswordMode(false)
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
        }
        // enter
        else if (event.text.unicode == 13)
        {
            value = input;
            isFocused = false;
            box.setOutlineColor(sf::Color::Black);
            _startNetwork(getIpFromString(input).c_str());
            display = false;
        }
        else if (event.text.unicode > 32 && event.text.unicode < 128)
        {
            input += static_cast<char>(event.text.unicode);
        }

        text.setString(input);
        // Update value in real-time so getText() returns current input
        value = input;
    }
}

void TextBox::setPasswordMode(bool enable)
{
    isPasswordMode = enable;
}

void TextBox::draw(sf::RenderWindow &window)
{
    window.draw(box);

    // If in password mode, display asterisks instead of actual text
    if (isPasswordMode && !input.empty())
    {
        sf::Text maskedText = text;
        std::string maskedString(input.length(), '*');
        maskedText.setString(maskedString);
        window.draw(maskedText);
    }
    else
    {
        window.draw(text);
    }
}

std::string TextBox::getText() const
{
    // Return input for real-time access to what user is typing
    return input;
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