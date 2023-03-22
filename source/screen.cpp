#include "screen.hpp"
#include <sys/time.h>

Screen::Screen(const char* title, uint32_t width, uint32_t height) : width(width), height(height)
{
    SDL_Init(SDL_INIT_EVERYTHING);

    window = SDL_CreateWindow(title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width,
                              height, SDL_WINDOW_SHOWN);

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);

    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_TARGET, width,
                                height);

    framebuffer = std::make_unique<uint8_t[]>(width * height * channels);
}

Screen::~Screen()
{
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

long long timeInMilliseconds(void)
{
    struct timeval tv;

    gettimeofday(&tv, NULL);
    return (((long long)tv.tv_sec) * 1000) + (tv.tv_usec / 1000);
}

bool Screen::loop()
{
    static long long last_updated = timeInMilliseconds();
    if (timeInMilliseconds() - last_updated > 33)
    {
        SDL_Event e;
        while (SDL_PollEvent(&e))
        {
            if (e.type == SDL_QUIT)
            {
                return false;
            }
        }

        last_updated = timeInMilliseconds();
    }

    return true;
}

uint8_t* Screen::get_framebuffer()
{
    return framebuffer.get();
}

void Screen::update_screen()
{
    SDL_UpdateTexture(texture, NULL, framebuffer.get(), width * sizeof(uint32_t));

    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);
}
