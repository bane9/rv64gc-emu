#include "clint.hpp"
#include "cpu.hpp"
#include "helper.hpp"
#include <fmt/core.h>

uint64_t ClintDevice::load(Bus& bus, uint64_t address, uint64_t length)
{
    uint64_t reg_value = 0;
    uint64_t offset = 0;

    if (helper::value_in_range_inclusive(address, msip_addr, msip_addr + sizeof(msip)))
    {
        reg_value = msip;
        offset = address - msip_addr;
    }
    else if (helper::value_in_range_inclusive(address, mtimecmp_addr,
                                              mtimecmp_addr + sizeof(mtimecmp)))
    {
        reg_value = mtimecmp;
        offset = address - mtimecmp_addr;
    }
    else if (helper::value_in_range_inclusive(address, mtime_addr, mtime_addr + sizeof(mtime)))
    {
        reg_value = mtime;
        offset = address - mtime_addr;
    }

    return (reg_value >> (offset * 8ULL)) & ((1ULL << length) - 1ULL);
}

void ClintDevice::store(Bus& bus, uint64_t address, uint64_t value, uint64_t length)
{
    uint64_t reg_value = 0;
    uint64_t offset = 0;

    if (helper::value_in_range_inclusive(address, msip_addr, msip_addr + sizeof(msip)))
    {
        reg_value = msip;
        offset = address - msip_addr;
    }
    else if (helper::value_in_range_inclusive(address, mtimecmp_addr,
                                              mtimecmp_addr + sizeof(mtimecmp)))
    {
        reg_value = mtimecmp;
        offset = address - mtimecmp_addr;
    }
    else if (helper::value_in_range_inclusive(address, mtime_addr, mtime_addr + sizeof(mtime)))
    {
        reg_value = mtime;
        offset = address - mtime_addr;
    }

    if (length != 64)
    {
        uint64_t mask = (1ULL << length) - 1ULL;

        reg_value &= ~(mask << (offset * 8));
        reg_value |= (value & mask) << (offset * 8);
    }

    if (helper::value_in_range_inclusive(address, msip_addr, msip_addr + sizeof(msip)))
    {
        msip = reg_value;
    }
    else if (helper::value_in_range_inclusive(address, mtimecmp_addr,
                                              mtimecmp_addr + sizeof(mtimecmp)))
    {
        mtimecmp = reg_value;
    }
    else if (helper::value_in_range_inclusive(address, mtime_addr, mtime_addr + sizeof(mtime)))
    {
        mtime = reg_value;
    }
}

void ClintDevice::tick(Cpu& cpu)
{
    ++mtime;
    cpu.cregs.store(csr::Address::TIME, cpu.cregs.load(csr::Address::TIME) + 1);

    if ((msip & 1) != 0)
    {
        cpu.cregs.store(csr::Address::MIP, cpu.cregs.load(csr::Address::MIP) | csr::Mask::MSIP);
    }

    if (mtime >= mtimecmp)
    {
        cpu.cregs.store(csr::Address::MIP, cpu.cregs.load(csr::Address::MIP) | csr::Mask::MTIP);
    }
    else
    {
        cpu.cregs.store(csr::Address::MIP, cpu.cregs.load(csr::Address::MIP) & ~csr::Mask::MTIP);
    }
}

uint64_t ClintDevice::get_base_address() const
{
    return base_addr;
}

uint64_t ClintDevice::get_end_address() const
{
    return end_addr;
}

void ClintDevice::dump(std::ostream& stream) const
{
    stream << fmt::format("msip = 0x{:0>4x}\n", msip);
    stream << fmt::format("mtimecmp = 0x{:0>8x}\n", mtimecmp);
    stream << fmt::format("mtime = 0x{:0>8x}", mtime);
}

std::string_view ClintDevice::get_peripheral_name() const
{
    return peripheral_name;
}