#pragma once

#include <SDL2/SDL.h>
#include <memory>
#include <stdint.h>

class Screen
{
  public:
    Screen(const char* title, uint32_t width, uint32_t height);
    ~Screen();

    bool loop();
    uint8_t* get_framebuffer();
    void update_screen();

  private:
    SDL_Renderer* renderer;
    SDL_Window* window;
    SDL_Texture* texture;

    uint32_t width;
    uint32_t height;
    uint32_t channels = 4;

    uint32_t last_updated = 0;

    std::unique_ptr<uint8_t[]> framebuffer;
};