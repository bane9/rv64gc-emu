#include "plic.hpp"
#include "helper.hpp"
#include <fmt/core.h>

uint64_t PlicDevice::load(Bus& bus, uint64_t address, uint64_t length)
{
    if (helper::value_in_range(address, source_prio_addr, source_prio_end_addr))
    {
        uint64_t idx = (address - source_prio_addr) / sizeof(prioprity[0]);

        return prioprity[idx];
    }
    else if (helper::value_in_range(address, pending_addr, pending_end_addr))
    {
        uint64_t idx = (address - pending_addr) / sizeof(prioprity[0]);

        return prioprity[idx];
    }
    else if (helper::value_in_range(address, enable_addr, enable_end_addr))
    {
        uint64_t idx = (address - enable_addr) / sizeof(prioprity[0]);

        return prioprity[idx];
    }
    else if (helper::value_in_range(address, treshold_claim_addr, treshold_claim_end_addr))
    {
        uint64_t idx = (address - treshold_claim_addr) / sizeof(prioprity[0]);

        return prioprity[idx];
    }

    return 0;
}

void PlicDevice::store(Bus& bus, uint64_t address, uint64_t value, uint64_t length)
{
    if (helper::value_in_range(address, source_prio_addr, source_prio_end_addr))
    {
        uint64_t idx = (address - source_prio_addr) / sizeof(prioprity[0]);

        prioprity[idx] = value;
    }
    else if (helper::value_in_range(address, pending_addr, pending_end_addr))
    {
        uint64_t idx = (address - pending_addr) / sizeof(prioprity[0]);

        prioprity[idx] = value;
    }
    else if (helper::value_in_range(address, enable_addr, enable_end_addr))
    {
        uint64_t idx = (address - enable_addr) / sizeof(prioprity[0]);

        prioprity[idx] = value;
    }
    else if (helper::value_in_range(address, treshold_claim_addr, treshold_claim_end_addr))
    {
        uint64_t idx = (address - treshold_claim_addr) / sizeof(prioprity[0]);

        prioprity[idx] = value;
    }
}

void PlicDevice::update_pending(uint64_t irq)
{
    uint64_t idx = irq / sizeof(pending[0]);

    pending[idx] |= 1 << irq;

    update_claim(irq);
}

void PlicDevice::clear_pending(uint64_t irq)
{
    uint64_t idx = irq / sizeof(pending[0]);

    pending[idx] &= ~(1 << irq);

    update_claim(irq);
}

void PlicDevice::update_claim(uint64_t irq)
{
    if (!irq || is_enabled(1, irq))
    {
        claim[1] = irq;
    }
}

bool PlicDevice::is_enabled(uint64_t context, uint64_t irq)
{
    uint64_t idx = (irq - prioprity.size()) / (sizeof(prioprity[0]) * 8);
    uint64_t offset = (irq - prioprity.size()) % (sizeof(prioprity[0]) * 8);

    return (enable[(context * 32 + idx)] >> offset) & 1;
}

uint64_t PlicDevice::get_base_address() const
{
    return base_addr;
}

uint64_t PlicDevice::get_end_address() const
{
    return end_addr;
}

void PlicDevice::dump(std::ostream& stream) const
{
}

std::string_view PlicDevice::get_peripheral_name() const
{
    return peripheral_name;
}
