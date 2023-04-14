#include "cpu.hpp"
#include "cpu_config.hpp"
#include "gpu.hpp"
#include "helper.hpp"
#include <iostream>
#include <optional>

namespace gpu
{

GpuDevice::GpuDevice(const char* screen_title, const char* font_path, uint32_t width,
                     uint32_t height)
{
    thread_done = false;

#if !CPU_TEST
    stdin_reader_thread = std::thread(&GpuDevice::stdin_reader, this);
#endif
}

GpuDevice::~GpuDevice()
{
#if !CPU_TEST
    thread_done = true;

    stdin_reader_thread.join();
#endif
}

uint64_t GpuDevice::load(Bus& bus, uint64_t address, uint64_t length)
{
    if (helper::value_in_range(address, uart_base_addr, uart_base_addr + uart_size))
    {
        if (address == cfg::lsr)
        {
            return 0x60;
        }

        return uart_data[address - uart_base_addr];
    }

    return 0;
}

void GpuDevice::store(Bus& bus, uint64_t address, uint64_t value, uint64_t length)
{
    if (helper::value_in_range(address, uart_base_addr, uart_base_addr + uart_size))
    {
        uart_data[address - uart_base_addr] = value;

        if (address == cfg::thr)
        {
            char c = static_cast<char>(value);
            std::cout << c;
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

#ifdef _WIN32

#include <windows.h>

void GpuDevice::stdin_reader()
{
    while (!thread_done)
    {
        if (_kbhit())
        {
            char c = _getch();
            read_char.store(c, std::memory_order::release);
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

#else // POSIX

#include <sys/select.h>
#include <termios.h>
#include <unistd.h>

void GpuDevice::stdin_reader()
{
    termios original_term, new_term;
    tcgetattr(STDIN_FILENO, &original_term);
    new_term = original_term;
    new_term.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &new_term);

    while (!thread_done)
    {
        fd_set read_fds;
        FD_ZERO(&read_fds);
        FD_SET(STDIN_FILENO, &read_fds);
        timeval timeout = {0, 10000}; // 10 ms

        if (select(STDIN_FILENO + 1, &read_fds, nullptr, nullptr, &timeout) > 0)
        {
            char c = getchar();
            read_char.store(c, std::memory_order::release);
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    tcsetattr(STDIN_FILENO, TCSANOW, &original_term);
}

#endif

void GpuDevice::tick(Cpu& cpu)
{
    char c = read_char.exchange('\0');

    if (c != '\0')
    {
        uart_data[0] = c;
        uart_data[cfg::lsr - uart_base_addr] |= cfg::lsr_rx;
        is_uart_interrupting = true;
    }
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