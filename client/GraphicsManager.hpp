#ifndef GRAPHICS_HPP
#define GRAPHICS_HPP

#include <SDL2/SDL.h>
#include <string>
#include <unordered_map>

class GraphicsManager {
private:
  SDL_Window *window;
  SDL_Renderer *renderer;
  std::unordered_map<std::string, SDL_Texture *> textures;

public:
  GraphicsManager();
  ~GraphicsManager();

  bool init(const char *title, int width, int height);
  void clear();
  void present();
  SDL_Texture *createColorTexture(int width, int height, Uint8 r, Uint8 g,
                                  Uint8 b, Uint8 a = 255);
  void storeTexture(const std::string &name, SDL_Texture *texture);
  SDL_Texture *getTexture(const std::string &name);
  void drawTexture(SDL_Texture *texture, int x, int y, int w, int h);
  void drawRect(int x, int y, int w, int h, Uint8 r, Uint8 g, Uint8 b,
                Uint8 a = 255);
  void drawText(const std::string &text, int x, int y, Uint8 r = 255,
                Uint8 g = 255, Uint8 b = 255);
};

// Global graphics manager
extern GraphicsManager *g_graphics;

#endif // GRAPHICS_HPP
