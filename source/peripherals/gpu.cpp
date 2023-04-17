#include "gpu.hpp"
#include "cpu.hpp"
#include "cpu_config.hpp"
#include "helper.hpp"
#include "terminal.hpp"
#include <iostream>
#include <optional>

namespace gpu
{

GpuDevice::GpuDevice(const char* screen_title, const char* font_path, uint32_t width,
                     uint32_t height)
{
    this->width = width;
    this->height = height;
    channels = 3;

    atexit(SDL_Quit); // There are several event/exception related exit points
    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();

    // SDL_ShowCursor(SDL_DISABLE);

    font = TTF_OpenFont(font_path, 48);

    window = SDL_CreateWindow(screen_title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width,
                              height, SDL_WINDOW_SHOWN);

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    texture =
        SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_TARGET, width, height);

    framebuffer = std::make_unique<uint8_t[]>(width * height * channels);
    fb_end += width * height * channels;

    terminal = std::make_unique<Terminal>(term_rows, term_cols, font);

    lsr = cfg::lsr_temt | cfg::lsr_thre;
    isr = 0xc0 | cfg::isr_no_int;
}

GpuDevice::~GpuDevice()
{
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_CloseFont(font);
}

uint64_t GpuDevice::load(Bus& bus, uint64_t address, uint64_t length)
{
    switch (address)
    {
    case fb_channels:
        return channels;
    case fb_dimensions:
        return (height << 16) | width;
    case term_dimensions:
        return (term_rows << 16) | term_cols;
    default:
        if (helper::value_in_range(address, fb_start, fb_end))
        {
            address -= fb_start;

            switch (length)
            {
            case 8:
                return framebuffer[address];
            case 16:
                return *reinterpret_cast<uint16_t*>(framebuffer.get() + address);
            case 32:
                return *reinterpret_cast<uint32_t*>(framebuffer.get() + address);
            case 64:
                return *reinterpret_cast<uint64_t*>(framebuffer.get() + address);
            default:
                break;
            }
        }
        else if (helper::value_in_range(address, uart_base_addr, uart_base_addr + uart_size))
        {
            switch (address)
            {
            case cfg::thr: {
                if (lsr & cfg::lsr_dr)
                {
                    lsr &= ~cfg::lsr_dr;
                    dispatch_interrupt();
                }

                return val;
            }
            case cfg::ier:
                return ier;
            case cfg::isr:
                return isr;
            case cfg::lcr:
                return lcr;
            case cfg::mcr:
                return mcr;
            case cfg::lsr:
                return lsr;
            case cfg::msr:
                return msr;
            case cfg::scr:
                return scr;
            default:
                break;
            }
        }
    }

    return 0;
}

void GpuDevice::store(Bus& bus, uint64_t address, uint64_t value, uint64_t length)
{
    switch (address)
    {
    case fb_render:
        if (value == 1) [[likely]]
        {
            render_framebuffer();
        }
        break;
    case fb_dimensions: {
        if (length != 32)
        {
            return;
        }

        uint16_t new_width = value & 0xffff;
        uint16_t new_height = (value >> 16) & 0xffff;

        resize_screen(new_width, new_height);

        break;
    }
    case term_dimensions: {
        if (length != 32)
        {
            return;
        }

        uint16_t new_cols = value & 0xffff;
        uint16_t new_rows = (value >> 16) & 0xffff;

        terminal->reset(new_rows, new_cols, font);

        term_cols = new_cols;
        term_rows = new_rows;

        break;
    }
    default:
        if (helper::value_in_range(address, fb_start, fb_end)) [[likely]]
        {
            address -= fb_start;

            switch (length)
            {
            case 8:
                [[likely]] framebuffer[address] = value;
                break;
            case 16:
                *reinterpret_cast<uint16_t*>(framebuffer.get() + address) = value;
                break;
            case 32:
                *reinterpret_cast<uint32_t*>(framebuffer.get() + address) = value;
                break;
            case 64:
                *reinterpret_cast<uint64_t*>(framebuffer.get() + address) = value;
                break;
            default:
                break;
            }
        }
        else if (helper::value_in_range(address, uart_base_addr, uart_base_addr + uart_size))
        {
            switch (address)
            {
            case cfg::thr: {
                char c = static_cast<char>(value);
                terminal->input_write(&c, sizeof(c));
                text_last_bufferred = helper::get_milliseconds();

                if (c == '\n')
                {
                    c = '\r';
                    terminal->input_write(&c, sizeof(c));
                    render_textbuffer();
                }

                break;
            }
            case cfg::ier:
                ier = value;
                dispatch_interrupt();
                break;
            case cfg::fcr:
                fcr = value;
                break;
            case cfg::lcr:
                lcr = value;
                break;
            case cfg::mcr:
                mcr = value;
                break;
            case cfg::scr:
                scr = value;
                break;
            default:
                break;
            }
        }
    }
}

std::optional<uint32_t> GpuDevice::is_interrupting()
{
    if (is_uart_interrupting)
    {
        is_uart_interrupting = false;
        return cfg::uart_irqn;
    }

    return std::nullopt;
}

void GpuDevice::uart_putchar(uint8_t c)
{
    val = c;
    lsr |= cfg::lsr_dr;
    dispatch_interrupt();
}

void GpuDevice::dispatch_interrupt()
{
    isr |= 0xc0;

    if ((ier & cfg::ier_rdi) && (lsr & cfg::lsr_dr))
    {
        is_uart_interrupting = true;
        return;
    }
    else if ((ier & cfg::ier_thri) && (lsr & cfg::lsr_temt))
    {
        is_uart_interrupting = true;
        return;
    }
}

void GpuDevice::tick(Cpu& cpu)
{
    uint64_t current_tick = helper::get_milliseconds();

    if (current_tick - last_tick > 10)
    {
        last_tick = current_tick;
        SDL_Event e;

        while (SDL_PollEvent(&e))
        {
            switch (e.type)
            {
            case SDL_QUIT:
                exit(0);
                break;
            case SDL_TEXTINPUT:
                uart_putchar(e.text.text[0]);
                break;
            case SDL_KEYDOWN:
                if (e.key.keysym.sym == SDLK_RETURN || e.key.keysym.sym == SDLK_KP_ENTER)
                {
                    uart_putchar('\n');
                }
                break;
            case SDL_KEYUP:
                val = 0;
                break;
            default:
                break;
            }
        }
    }

    if (text_last_bufferred != ~0ULL && helper::get_milliseconds() - text_last_bufferred > 60)
        [[unlikely]]
    {
        render_textbuffer();
    }
}

void GpuDevice::render_framebuffer()
{
    SDL_UpdateTexture(texture, NULL, framebuffer.get(), width * channels);

    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);
}

void GpuDevice::render_textbuffer()
{
    text_last_bufferred = ~0ULL;
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    SDL_Rect rect = {};
    rect.w = width;
    rect.h = height;

    terminal->render(renderer, rect);
    SDL_RenderPresent(renderer);
}

void GpuDevice::resize_screen(uint16_t width, uint16_t height)
{
    this->width = width;
    this->height = height;

    SDL_SetWindowSize(window, width, height);

    uint32_t pixel_count = width * height * channels;

    fb_end = fb_start + pixel_count;

    framebuffer = std::make_unique<uint8_t[]>(pixel_count);

    SDL_DestroyTexture(texture);

    texture =
        SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_TARGET, width, height);
}

void GpuDevice::dump(std::ostream& stream) const
{
}

uint64_t GpuDevice::get_base_address() const
{
    return base_addr;
}

uint64_t GpuDevice::get_end_address() const
{
    return end_addr;
}

std::string_view GpuDevice::get_peripheral_name() const
{
    return peripheral_name;
}

} // namespace gpu