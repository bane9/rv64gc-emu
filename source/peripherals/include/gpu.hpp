#pragma once

#include "bus.hpp"
#include "cpu_config.hpp"
#include <memory>
#include <ostream>
#include <queue>
#include <string_view>

#if !NATIVE_CLI && !CPU_TEST
#include "terminal.hpp"
#include <SDL2/SDL.h>
#else
#include <atomic>
#include <thread>
#endif

namespace gpu
{

namespace cfg
{
constexpr uint64_t uart_base_address = 0x10000000ULL;
constexpr uint64_t uart_irqn = 10;

constexpr uint64_t rhr = uart_base_address + 0;
constexpr uint64_t thr = uart_base_address + 0;
constexpr uint64_t dll = uart_base_address + 0;

constexpr uint64_t ier = uart_base_address + 1;
constexpr uint64_t dlm = uart_base_address + 1;

constexpr uint64_t isr = uart_base_address + 2;
constexpr uint64_t fcr = uart_base_address + 2;

constexpr uint64_t lcr = uart_base_address + 3;
constexpr uint64_t mcr = uart_base_address + 4;
constexpr uint64_t lsr = uart_base_address + 5;
constexpr uint64_t msr = uart_base_address + 6;
constexpr uint64_t scr = uart_base_address + 7;

constexpr uint8_t lsr_dr = 0x1;
constexpr uint8_t lsr_thre = 0x20;
constexpr uint8_t lsr_temt = 0x40;

constexpr uint8_t ier_rdi = 0x01;
constexpr uint8_t ier_thri = 0x02;

constexpr uint8_t lcr_dlab = 0x80;

constexpr uint8_t isr_no_int = 0x01;
constexpr uint8_t isr_thri = 0x02;
constexpr uint8_t isr_rdi = 0x04;

}; // namespace cfg

class GpuDevice : public BusDevice
{
  public:
    GpuDevice(const char* screen_title, const char* font_path, uint32_t width, uint32_t height);
    virtual ~GpuDevice();

    uint64_t load(Bus& bus, uint64_t address, uint64_t length) override;
    void store(Bus& bus, uint64_t address, uint64_t value, uint64_t length) override;

  public:
    void tick(Cpu& cpu) override;
    std::optional<uint32_t> is_interrupting() override;

  public:
    uint64_t get_base_address() const override;
    uint64_t get_end_address() const override;

    void dump(std::ostream& stream) const override;

    std::string_view get_peripheral_name() const override;

  public:
    static constexpr uint64_t uart_base_addr = cfg::uart_base_address;

    static constexpr uint64_t uart_size = 0x100;

    static constexpr uint64_t fb_render = uart_base_addr - 0x700000;
    static constexpr uint64_t fb_dimensions = fb_render + sizeof(uint32_t);
    static constexpr uint64_t term_dimensions = fb_dimensions + sizeof(uint32_t);
    static constexpr uint64_t fb_channels = term_dimensions + sizeof(uint32_t);

    static constexpr uint64_t fb_start = fb_channels + sizeof(uint32_t);
    uint64_t fb_end = fb_start;

    static constexpr uint64_t base_addr = fb_render;

    static constexpr uint64_t end_addr = cfg::uart_base_address + uart_size;

    static constexpr std::string_view peripheral_name = "GPU";

#if !NATIVE_CLI && !CPU_TEST
  public:
    void uart_putchar(uint8_t c);
    void render_textbuffer();
    void resize_screen(uint16_t width, uint16_t height);
    void render_framebuffer();

  public:
    SDL_Renderer* renderer;
    SDL_Window* window;
    SDL_Texture* texture;

    uint32_t width;
    uint32_t height;
    uint32_t channels;

    uint64_t last_tick = 0;

    uint64_t text_last_bufferred = ~0ULL;

    std::unique_ptr<uint8_t[]> framebuffer;

  public:
    std::unique_ptr<Terminal> terminal;
    TTF_Font* font;

    int term_rows = 32;
    int term_cols = 120;
#else
  public:
    bool thread_done = false;
    std::atomic<char> read_char;
    void stdin_reader();
#if __EMSCRIPTEN__
    pthread_t thread;
#else
    std::thread stdin_reader_thread;
#endif
#endif

  public:
    void dispatch_interrupt();
    bool is_uart_interrupting = false;

  public:
    uint8_t dll = 0;
    uint8_t dlm = 0;
    uint8_t isr = 0;
    uint8_t ier = 0;
    uint8_t fcr = 0;
    uint8_t lcr = 0;
    uint8_t mcr = 0;
    uint8_t lsr = 0;
    uint8_t msr = 0;
    uint8_t scr = 0;
    uint8_t val = 0;
};
} // namespace gpu