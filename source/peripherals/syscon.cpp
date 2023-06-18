#include "syscon.hpp"
#include <cstdlib>

uint64_t SysconDevice::load(Bus& bus, uint64_t address, uint64_t length)
{
    return 0;
}

void SysconDevice::store(Bus& bus, uint64_t address, uint64_t value, uint64_t length)
{
    if (value == poweroff_addr)
    {
        std::exit(0);
    }
    else if (value == reboot_addr)
    {
        std::exit(0);
    }
}

void SysconDevice::dump(std::ostream& stream) const
{
}

uint64_t SysconDevice::get_base_address() const
{
    return base_addr;
}

uint64_t SysconDevice::get_end_address() const
{
    return end_addr;
}

std::string_view SysconDevice::get_peripheral_name() const
{
    return peripheral_name;
}
