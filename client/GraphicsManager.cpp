#include "GraphicsManager.hpp"
#include <iostream>

GraphicsManager *g_graphics = nullptr;

GraphicsManager::GraphicsManager() : window(nullptr), renderer(nullptr) {}

GraphicsManager::~GraphicsManager() {
  if (renderer)
    SDL_DestroyRenderer(renderer);
  if (window)
    SDL_DestroyWindow(window);
  SDL_Quit();
}

bool GraphicsManager::init(const char *title, int width, int height) {
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError()
              << std::endl;
    return false;
  }

  window =
      SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                       width, height, SDL_WINDOW_SHOWN);
  if (!window) {
    std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError()
              << std::endl;
    return false;
  }

  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
  if (!renderer) {
    std::cerr << "Renderer could not be created! SDL_Error: " << SDL_GetError()
              << std::endl;
    return false;
  }

  return true;
}

void GraphicsManager::clear() {
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
  SDL_RenderClear(renderer);
}

void GraphicsManager::present() { SDL_RenderPresent(renderer); }

SDL_Texture *GraphicsManager::createColorTexture(int width, int height, Uint8 r,
                                                 Uint8 g, Uint8 b, Uint8 a) {
  SDL_Texture *texture =
      SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888,
                        SDL_TEXTUREACCESS_TARGET, width, height);
  if (!texture) {
    std::cerr << "Failed to create texture! SDL_Error: " << SDL_GetError()
              << std::endl;
    return nullptr;
  }

  SDL_SetRenderTarget(renderer, texture);
  SDL_SetRenderDrawColor(renderer, r, g, b, a);
  SDL_RenderClear(renderer);
  SDL_SetRenderTarget(renderer, nullptr);

  return texture;
}

void GraphicsManager::storeTexture(const std::string &name,
                                   SDL_Texture *texture) {
  textures[name] = texture;
}

SDL_Texture *GraphicsManager::getTexture(const std::string &name) {
  return textures[name];
}

void GraphicsManager::drawTexture(SDL_Texture *texture, int x, int y, int w,
                                  int h) {
  SDL_Rect dstRect = {x, y, w, h};
  SDL_RenderCopy(renderer, texture, nullptr, &dstRect);
}

void GraphicsManager::drawRect(int x, int y, int w, int h, Uint8 r, Uint8 g,
                               Uint8 b, Uint8 a) {
  SDL_SetRenderDrawColor(renderer, r, g, b, a);
  SDL_Rect rect = {x, y, w, h};
  SDL_RenderFillRect(renderer, &rect);
}

void GraphicsManager::drawText(const std::string &text, int x, int y, Uint8 r,
                               Uint8 g, Uint8 b) {
  // Placeholder for text rendering (requires SDL_ttf for actual implementation)
  // std::cerr << "Text rendering not implemented: " << text << std::endl;
}