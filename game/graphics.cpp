#include "graphics.hpp"
#include <iostream>

GraphicsManager* g_graphics = nullptr;

GraphicsManager::GraphicsManager() : window(nullptr), renderer(nullptr), windowWidth(0), windowHeight(0) {}

GraphicsManager::~GraphicsManager() {
    cleanup();
}

bool GraphicsManager::init(const char* title, int width, int height) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL could not initialize! SDL Error: " << SDL_GetError() << std::endl;
        return false;
    }
    
    window = SDL_CreateWindow(title,
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        width, height, SDL_WINDOW_SHOWN);
    
    if (window == nullptr) {
        std::cerr << "Window could not be created! SDL Error: " << SDL_GetError() << std::endl;
        return false;
    }
    
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (renderer == nullptr) {
        std::cerr << "Renderer could not be created! SDL Error: " << SDL_GetError() << std::endl;
        return false;
    }
    
    windowWidth = width;
    windowHeight = height;
    
    // Initialize PNG loading
    int imgFlags = IMG_INIT_PNG;
    if (!(IMG_Init(imgFlags) & imgFlags)) {
        std::cerr << "SDL_image could not initialize! SDL_image Error: " << IMG_GetError() << std::endl;
        return false;
    }
    if (TTF_Init() == -1) {
        std::cerr << "SDL_ttf could not initialize! SDL_ttf Error: " << TTF_GetError() << std::endl;
        return false;
    }

    font = TTF_OpenFont("../assets/fonts/upheavtt.ttf", 24);
    if (!font) {
        std::cerr << "Failed to load font! SDL_ttf Error: " << TTF_GetError() << std::endl;
        return false;
    }

    return true;
}

void GraphicsManager::cleanup() {
    // Clean up textures
    for (auto& pair : textures) {
        SDL_DestroyTexture(pair.second);
    }
    textures.clear();
    
    if (renderer) {
        SDL_DestroyRenderer(renderer);
        renderer = nullptr;
    }
    
    if (window) {
        SDL_DestroyWindow(window);
        window = nullptr;
    }
    
    IMG_Quit();
    SDL_Quit();
}

SDL_Texture* GraphicsManager::loadTexture(const std::string& path, const std::string& name) {
    SDL_Texture* texture = IMG_LoadTexture(renderer, path.c_str());
    if (texture == nullptr) {
        std::cerr << "Unable to load image " << path << "! SDL_image Error: " << IMG_GetError() << std::endl;
        return nullptr;
    }
    
    textures[name] = texture;
    return texture;
}

SDL_Texture* GraphicsManager::createColorTexture(int width, int height, Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
    SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, width, height);
    
    if (texture) {
        SDL_SetRenderTarget(renderer, texture);
        SDL_SetRenderDrawColor(renderer, r, g, b, a);
        SDL_RenderClear(renderer);
        SDL_SetRenderTarget(renderer, nullptr);
        SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
    }
    
    return texture;
}

void GraphicsManager::storeTexture(const std::string& name, SDL_Texture* texture) {
    textures[name] = texture;
}

void GraphicsManager::clear() {
    SDL_SetRenderDrawColor(renderer, 135, 206, 235, 255); // Sky blue background
    SDL_RenderClear(renderer);
}

void GraphicsManager::present() {
    SDL_RenderPresent(renderer);
}

void GraphicsManager::drawTexture(SDL_Texture* texture, int x, int y, int width, int height) {
    if (!texture) return;
    
    SDL_Rect destRect = {x, y, width, height};
    
    if (width == 0 || height == 0) {
        SDL_QueryTexture(texture, nullptr, nullptr, &destRect.w, &destRect.h);
    }
    
    SDL_RenderCopy(renderer, texture, nullptr, &destRect);
}

void GraphicsManager::drawRect(int x, int y, int width, int height, Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
    SDL_SetRenderDrawColor(renderer, r, g, b, a);
    SDL_Rect rect = {x, y, width, height};
    SDL_RenderFillRect(renderer, &rect);
}

void GraphicsManager::drawText(const std::string& text, int x, int y, Uint8 r, Uint8 g, Uint8 b) {
    if (!font) return;

    SDL_Color color = {r, g, b};
    SDL_Surface* textSurface = TTF_RenderText_Solid(font, text.c_str(), color);
    if (!textSurface) {
        std::cerr << "Unable to render text surface! SDL_ttf Error: " << TTF_GetError() << std::endl;
        return;
    }

    // Convert the surface to a texture
    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    if (!textTexture) {
        std::cerr << "Unable to create texture from rendered text! SDL Error: " << SDL_GetError() << std::endl;
        SDL_FreeSurface(textSurface);
        return;
    }

    // Define the destination rectangle for rendering the text
    SDL_Rect destRect = {x, y, textSurface->w, textSurface->h};

    // Render the texture to the screen
    SDL_RenderCopy(renderer, textTexture, nullptr, &destRect);

    // Clean up
    SDL_DestroyTexture(textTexture);
    SDL_FreeSurface(textSurface);
}

SDL_Texture* GraphicsManager::getTexture(const std::string& name) {
    auto it = textures.find(name);
    return (it != textures.end()) ? it->second : nullptr;
}