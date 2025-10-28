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


#define NOMINMAX

#include "GraphicsManager.hpp"
#include "../client/NetworkECSMediator.hpp"
#include "../client/windowSize.hpp"
#include "../client/assetsPath.hpp"
#include "textBox.hpp"
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
#include <SFML/Graphics/Font.hpp>
#include <iostream>
#include <memory>

GraphicsManager *g_graphics = nullptr;

GraphicsManager::GraphicsManager(NetworkECSMediator med) : _med(med), 
    _showError(false),
    _errorMessage("")
{
}

bool GraphicsManager::registerTheTexture()
{
    createTextureFromPath(PathFormater::formatAssetPath("assets/sprites/background.jpg"), "background");
    createTextureFromPath(PathFormater::formatAssetPath("assets/sprites/playerSpritesheet.png"), "player");

    return true;
}

GraphicsManager::~GraphicsManager()
{
}

bool GraphicsManager::init(const std::string &title, int width, int height,
                           std::function<void(const char *)> startNetwork)
{
    window.create(sf::VideoMode(width, height), title, sf::Style::Close);
    registerTheTexture();
    registerTheSound();
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
    case BONUS_LIFE:
        return getTexture("bonus_life");
    case BOSS:
        return getTexture("boss");
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

void GraphicsManager::drawText(const std::string &content, float x, float y, sf::Uint8 r, sf::Uint8 g, sf::Uint8 b, bool centered, int textSize)
{
    sf::Text text;
    text.setFont(font);
    text.setString(content);
    text.setCharacterSize(textSize);
    text.setFillColor(sf::Color(r, g, b));

    if (centered)
    {
        sf::FloatRect textBounds = text.getLocalBounds();
        x -= textBounds.width / 2.0f;
        y -= textBounds.height / 2.0f;
    }
    text.setPosition((float)x, (float)y);

    window.draw(text);
}

sf::RenderWindow &GraphicsManager::getWindow()
{
    return window;
}

std::unique_ptr<TextBox> &GraphicsManager::getTextBox()
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

bool GraphicsManager::registerTheSound()
{
    createSoundFromPath(PathFormater::formatAssetPath("assets/sounds/pew.mp3"), "pew");
    createSoundFromPath(PathFormater::formatAssetPath("assets/sounds/music.mp3"), "music");

    return true;
}

sf::Sound &GraphicsManager::createSoundFromPath(const std::string &filePath, const std::string &name)
{
    sf::SoundBuffer buffer;
    if (!buffer.loadFromFile(filePath))
    {
        std::cerr << "Error: SFML Failed to load sound from file: " << filePath << std::endl;
        throw std::runtime_error("Failed to load sound: " + filePath);
    }

    soundBuffers[name] = buffer;
    sf::Sound sound;
    sound.setBuffer(soundBuffers[name]);
    sounds[name] = sound;

    return sounds[name];
}

sf::Sound *GraphicsManager::getSound(const std::string &name)
{
    auto it = sounds.find(name);
    if (it != sounds.end())
        return &it->second;
    return nullptr;
}

void GraphicsManager::playSound(const std::string &name, bool loop)
{
    auto it = sounds.find(name);
    if (it != sounds.end())
    {
        it->second.setLoop(loop);
        it->second.play();
    }
    else
    {
        std::cerr << "Warning: Sound not found: " << name << std::endl;
    }
}

void GraphicsManager::stopSound(const std::string &name)
{
    auto it = sounds.find(name);
    if (it != sounds.end())
    {
        it->second.stop();
    }
}

void GraphicsManager::initLoginMenu()
{
    if (menuInitialized)
        return;
    // Background
    menuBackground.setSize(sf::Vector2f(window.getSize().x, window.getSize().y));
    menuBackground.setFillColor(sf::Color(30, 30, 30, 200));

    // Title
    menuTitle.setFont(font);
    menuTitle.setString("R-TYPE");
    menuTitle.setCharacterSize(72);
    menuTitle.setFillColor(sf::Color::White);
    menuTitle.setPosition(window.getSize().x / 2.0f - 100, 100);

    // Username textbox
    _usernameTextbox = std::make_unique<TextBox>(font, [](const char *) {}, 300, 40);
    _usernameTextbox->setPosition(window.getSize().x / 2.0f - 150, 200);

    // Password textbox
    _passwordTextbox = std::make_unique<TextBox>(font, [](const char *) {}, 300, 40);
    _passwordTextbox->setPosition(window.getSize().x / 2.0f - 150, 270);
    _passwordTextbox->setPasswordMode(true);

    // Signin Button (formerly Play)
    loginButton.setSize(sf::Vector2f(200, 60));
    loginButton.setFillColor(sf::Color(70, 130, 180));
    loginButton.setPosition(window.getSize().x / 2.0f - 100, 340);

    loginButtonText.setFont(font);
    loginButtonText.setString("LOGIN");
    loginButtonText.setCharacterSize(30);
    loginButtonText.setFillColor(sf::Color::White);
    // loginButtonText.setPosition(window.getSize().x / 2.0f - 60, 355);
    sf::FloatRect loginTextBounds = loginButtonText.getLocalBounds();
    loginButtonText.setPosition(
        loginButton.getPosition().x + (loginButton.getSize().x - loginTextBounds.width) / 2,
        loginButton.getPosition().y + (loginButton.getSize().y - loginTextBounds.height) / 2 - loginTextBounds.top
    );

    // Login Button (formerly Quit)
    signinButton.setSize(sf::Vector2f(200, 60));
    signinButton.setFillColor(sf::Color(180, 70, 70));
    signinButton.setPosition(window.getSize().x / 2.0f - 100, 420);

    signinButtonText.setFont(font);
    signinButtonText.setString("SIGNIN");
    signinButtonText.setCharacterSize(30);
    signinButtonText.setFillColor(sf::Color::White);
    // signinButtonText.setPosition(window.getSize().x / 2.0f - 50, 435);
    sf::FloatRect signinTextBounds = signinButtonText.getLocalBounds();
    signinButtonText.setPosition(
        signinButton.getPosition().x + (signinButton.getSize().x - signinTextBounds.width) / 2,
        signinButton.getPosition().y + (signinButton.getSize().y - signinTextBounds.height) / 2 - signinTextBounds.top
    );

    menuInitialized = true;
}

void GraphicsManager::drawMenu()
{
    if (!menuInitialized)
        initLoginMenu();

    window.draw(menuBackground);
    window.draw(menuTitle);

    // Draw textbox labels
    sf::Text usernameLabel;
    usernameLabel.setFont(font);
    usernameLabel.setString("Username:");
    usernameLabel.setCharacterSize(20);
    usernameLabel.setFillColor(sf::Color::White);
    usernameLabel.setPosition(window.getSize().x / 2.0f - 150, 175);
    window.draw(usernameLabel);

    sf::Text passwordLabel;
    passwordLabel.setFont(font);
    passwordLabel.setString("Password:");
    passwordLabel.setCharacterSize(20);
    passwordLabel.setFillColor(sf::Color::White);
    passwordLabel.setPosition(window.getSize().x / 2.0f - 150, 245);
    window.draw(passwordLabel);

    // Draw textboxes
    _usernameTextbox->draw(window);
    _passwordTextbox->draw(window);

    window.draw(loginButton);
    window.draw(loginButtonText);
    window.draw(signinButton);
    window.draw(signinButtonText);
}

GraphicsManager::MenuAction GraphicsManager::handleMenuClick(int mouseX, int mouseY)
{
    sf::Vector2f mousePos(mouseX, mouseY);

    if (loginButton.getGlobalBounds().contains(mousePos))
    {
        return MenuAction::LOGIN;
    }

    if (signinButton.getGlobalBounds().contains(mousePos))
    {
        return MenuAction::SIGNIN;
    }

    return MenuAction::NONE;
}

void GraphicsManager::initLobbyMenuUI()
{
    if (lobbyMenuInitialized)
        return;

    // Background
    lobbyMenuBackground.setSize(sf::Vector2f(window.getSize().x, window.getSize().y));
    lobbyMenuBackground.setFillColor(sf::Color(30, 30, 30, 200));

    // Title
    lobbyMenuTitle.setFont(font);
    lobbyMenuTitle.setString("SELECT LOBBY");
    lobbyMenuTitle.setCharacterSize(60);
    lobbyMenuTitle.setFillColor(sf::Color::White);
    lobbyMenuTitle.setPosition(window.getSize().x / 2.0f - 200, 80);

    // Create Lobby Button
    createLobbyButton.setSize(sf::Vector2f(250, 60));
    createLobbyButton.setFillColor(sf::Color(70, 180, 130));
    createLobbyButton.setPosition(window.getSize().x / 2.0f - 125, 300);

    createLobbyButtonText.setFont(font);
    createLobbyButtonText.setString("CREATE LOBBY");
    createLobbyButtonText.setCharacterSize(28);
    createLobbyButtonText.setFillColor(sf::Color::White);
    createLobbyButtonText.setPosition(window.getSize().x / 2.0f - 100, 312);

    // Join Lobby Button
    joinLobbyButton.setSize(sf::Vector2f(250, 60));
    joinLobbyButton.setFillColor(sf::Color(70, 130, 180));
    joinLobbyButton.setPosition(window.getSize().x / 2.0f - 125, 400);

    joinLobbyButtonText.setFont(font);
    joinLobbyButtonText.setString("JOIN LOBBY");
    joinLobbyButtonText.setCharacterSize(28);
    joinLobbyButtonText.setFillColor(sf::Color::White);
    joinLobbyButtonText.setPosition(window.getSize().x / 2.0f - 90, 412);

    // Back Button
    backButton.setSize(sf::Vector2f(200, 50));
    backButton.setFillColor(sf::Color(180, 70, 70));
    backButton.setPosition(50, window.getSize().y - 100);

    backButtonText.setFont(font);
    backButtonText.setString("BACK");
    backButtonText.setCharacterSize(24);
    backButtonText.setFillColor(sf::Color::White);
    backButtonText.setPosition(105, window.getSize().y - 90);

    // Lobby name textbox
    _lobbyTextbox = std::make_unique<TextBox>(font, [](const char *) {}, 400, 50);
    _lobbyTextbox->setPosition(window.getSize().x / 2.0f - 200, 200);

    lobbyMenuInitialized = true;
}

void GraphicsManager::drawLobbyMenu()
{
    if (!lobbyMenuInitialized)
        initLobbyMenuUI();

    window.draw(lobbyMenuBackground);
    window.draw(lobbyMenuTitle);

    // Draw instruction text
    sf::Text instructionText;
    instructionText.setFont(font);
    instructionText.setString("Enter lobby name:");
    instructionText.setCharacterSize(24);
    instructionText.setFillColor(sf::Color::White);
    instructionText.setPosition(window.getSize().x / 2.0f - 200, 160);
    window.draw(instructionText);

    // Draw textbox
    _lobbyTextbox->draw(window);

    // Draw buttons
    window.draw(createLobbyButton);
    window.draw(createLobbyButtonText);
    window.draw(joinLobbyButton);
    window.draw(joinLobbyButtonText);
    window.draw(backButton);
    window.draw(backButtonText);
}

GraphicsManager::MenuAction GraphicsManager::handleLobbyMenuClick(int mouseX, int mouseY)
{
    sf::Vector2f mousePos(mouseX, mouseY);

    if (createLobbyButton.getGlobalBounds().contains(mousePos))
    {
        return MenuAction::CREATE_LOBBY;
    }

    if (joinLobbyButton.getGlobalBounds().contains(mousePos))
    {
        return MenuAction::JOIN_LOBBY;
    }

    if (backButton.getGlobalBounds().contains(mousePos))
    {
        return MenuAction::BACK;
    }

    return MenuAction::NONE;
}

std::unique_ptr<TextBox> &GraphicsManager::getLobbyTextBox()
{
    if (!lobbyMenuInitialized)
        initLobbyMenuUI();
    return _lobbyTextbox;
}

std::unique_ptr<TextBox> &GraphicsManager::getUsernameTextBox()
{
    if (!menuInitialized)
        initLoginMenu();
    return _usernameTextbox;
}

std::unique_ptr<TextBox> &GraphicsManager::getPasswordTextBox()
{
    if (!menuInitialized)
        initLoginMenu();
    return _passwordTextbox;
}

void GraphicsManager::showErrorMessage(const std::string& message)
{
    _errorMessage = message;
    _showError = true;
    _errorMessageClock.restart();
    
    std::cout << "Showing error message: " << message << std::endl;
}

void GraphicsManager::updateErrorMessage()
{
    // Check if we need to show an error message
    printf("show error: %d\n", _showError);
    if (_showError) {
        // Create a background rectangle
        sf::RectangleShape errorBackground;
        errorBackground.setSize(sf::Vector2f(600, 100));
        errorBackground.setFillColor(sf::Color(200, 0, 0, 220)); // Semi-transparent red
        errorBackground.setOutlineColor(sf::Color::White);
        errorBackground.setOutlineThickness(2);
        
        // Center the background on screen
        float bgX = (windowWidth - errorBackground.getSize().x) / 2;
        float bgY = (windowHeight - errorBackground.getSize().y) / 2;
        errorBackground.setPosition(bgX, bgY);
        
        // Draw the background
        window.draw(errorBackground);
        
        // Draw the error text
        sf::Text errorText;
        errorText.setFont(font);
        errorText.setString(_errorMessage);
        errorText.setCharacterSize(20);
        errorText.setFillColor(sf::Color::White);
        
        // Center the text on the background
        sf::FloatRect textBounds = errorText.getLocalBounds();
        float textX = bgX + (errorBackground.getSize().x - textBounds.width) / 2;
        float textY = bgY + (errorBackground.getSize().y - textBounds.height) / 2 - 10;
        errorText.setPosition(textX, textY);
        
        window.draw(errorText);
        
        // Add a dismissal instruction
        sf::Text dismissText;
        dismissText.setFont(font);
        dismissText.setString("Press any key to dismiss");
        dismissText.setCharacterSize(15);
        dismissText.setFillColor(sf::Color(220, 220, 220));
        
        // Position below the error message
        sf::FloatRect dismissBounds = dismissText.getLocalBounds();
        float dismissX = bgX + (errorBackground.getSize().x - dismissBounds.width) / 2;
        float dismissY = textY + textBounds.height + 15;
        dismissText.setPosition(dismissX, dismissY);
        
        window.draw(dismissText);
        
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
            if (_showError && (event.type == sf::Event::KeyPressed || event.type == sf::Event::MouseButtonPressed)) {
                printf("SHOW ERROR: FALSE\n");
                _showError = false;
                break;
            }
        }
    }
}