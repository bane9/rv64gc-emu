#include "cpu.hpp"
#include "cpu_config.hpp"
#include "gpu.hpp"
#include "helper.hpp"
#include <iostream>
#include <optional>

namespace gpu
{

#if __EMSCRIPTEN__
void* emscripten_thread(void* arg)
{
    GpuDevice* device = static_cast<GpuDevice*>(arg);

    device->stdin_reader();

    return nullptr;
}
#endif

GpuDevice::GpuDevice(const char* screen_title, const char* font_path, uint32_t width,
                     uint32_t height)
{
    thread_done = false;

    lsr = cfg::lsr_temt | cfg::lsr_thre;
    isr = 0xc0 | cfg::isr_no_int;

#if !CPU_TEST && !__EMSCRIPTEN__
    stdin_reader_thread = std::thread(&GpuDevice::stdin_reader, this);
#elif __EMSCRIPTEN__
    pthread_create(&thread, nullptr, emscripten_thread, this);
#endif
}

GpuDevice::~GpuDevice()
{
    thread_done = true;

#if !CPU_TEST && !__EMSCRIPTEN__
    // stdin_reader_thread.join();
#elif __EMSCRIPTEN__
    // pthread_join(thread, nullptr);
#endif
}

uint64_t GpuDevice::load(Bus& bus, uint64_t address, uint64_t length)
{
    if (helper::value_in_range_inclusive(address, uart_base_addr, uart_base_addr + uart_size))
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

    return 0;
}

void GpuDevice::store(Bus& bus, uint64_t address, uint64_t value, uint64_t length)
{
    if (helper::value_in_range_inclusive(address, uart_base_addr, uart_base_addr + uart_size))
    {
        switch (address)
        {
        case cfg::thr: {
            char c = static_cast<char>(value);
            std::cout << c;
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

std::optional<uint32_t> GpuDevice::is_interrupting()
{
    if (is_uart_interrupting)
    {
        is_uart_interrupting = false;
        return cfg::uart_irqn;
    }

    return std::nullopt;
}

void GpuDevice::stdin_reader()
{
    while (!thread_done)
    {
        char c;
        std::cin >> c;

        read_char.store(c, std::memory_order::release);
    }
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
    char c = read_char.exchange('\0');

    if (c != '\0')
    {
        val = c;
        lsr |= cfg::lsr_dr;
        dispatch_interrupt();
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