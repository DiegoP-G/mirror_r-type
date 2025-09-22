#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <unordered_map>
#include <string>

class GraphicsManager {
private:
    SDL_Window* window;
    SDL_Renderer* renderer;
    std::unordered_map<std::string, SDL_Texture*> textures;
    int windowWidth;
    int windowHeight;
    TTF_Font* font;
    
public:
    GraphicsManager();
    ~GraphicsManager();
    
    bool init(const char* title, int width, int height);
    void cleanup();
    
    SDL_Texture* loadTexture(const std::string& path, const std::string& name);
    SDL_Texture* createColorTexture(int width, int height, Uint8 r, Uint8 g, Uint8 b, Uint8 a = 255);
    void storeTexture(const std::string& name, SDL_Texture* texture);
    
    void clear();
    void present();
    
    void drawTexture(SDL_Texture* texture, int x, int y, int width = 0, int height = 0);
    void drawRect(int x, int y, int width, int height, Uint8 r, Uint8 g, Uint8 b, Uint8 a = 255);
    void drawText(const std::string& text, int x, int y, Uint8 r = 255, Uint8 g = 255, Uint8 b = 255);
    
    SDL_Texture* getTexture(const std::string& name);
    SDL_Renderer* getRenderer() { return renderer; }
    
    int getWindowWidth() const { return windowWidth; }
    int getWindowHeight() const { return windowHeight; }
};

extern GraphicsManager* g_graphics;