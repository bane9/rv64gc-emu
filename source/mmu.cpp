#include "mmu.hpp"
#include "cpu.hpp"
#include "cpu_config.hpp"
#include "helper.hpp"
#include <cassert>
#include <utility>

namespace mmu
{

Mmu::Mmu(Cpu& cpu) : cpu(cpu)
{
    mode = Mode::Bare;
}

uint64_t Mmu::load(uint64_t address, uint64_t length)
{
    uint64_t p_address = translate(address, AccessType::Load);

    if (cpu.exc_val != exception::Exception::None)
    {
        return 0;
    }

    uint64_t value = cpu.bus.load(cpu, p_address, length);

    return value;
}

uint64_t Mmu::load_insn(uint64_t address, uint64_t length)
{
    uint64_t p_address = translate(address, AccessType::Instruction);

    if (cpu.exc_val != exception::Exception::None)
    {
        return 0;
    }

    uint64_t value = cpu.bus.load(cpu, p_address, length);

    if (cpu.exc_val == exception::Exception::LoadAccessFault)
    {
        cpu.exc_val = exception::Exception::InstructionAccessFault;
    }

    return value;
}

void Mmu::store(uint64_t address, uint64_t value, uint64_t length)
{
    uint64_t p_address = translate(address, AccessType::Store);

    if (cpu.exc_val != exception::Exception::None)
    {
        return;
    }

    cpu.bus.store(cpu, p_address, value, length);
}

void Mmu::update()
{
    uint64_t satp = cpu.cregs.load(csr::Address::SATP);

    uint64_t mode = helper::read_bits(satp, 63, 60);
    uint64_t ppn = helper::read_bits(satp, 43, 0);

    this->mppn = ppn << 12;
    this->mode = static_cast<Mode::ModeValue>(mode);

    assert(mode == Mode::Bare || mode == Mode::SV39 || mode == Mode::SV48 || mode == Mode::SV57);
}

uint32_t Mmu::get_levels()
{
    switch (mode)
    {
    case mmu::Mode::SV39:
        return 3;
    case mmu::Mode::SV48:
        return 4;
    case mmu::Mode::SV57:
        return 5;
    default:
        return 0;
    }
}

std::array<uint64_t, 5> Mmu::get_vpn(uint64_t address)
{
    std::array<uint64_t, 5> vpn = {};

    for (int i = 0; i < get_levels(); i++)
    {
        vpn[i] = (address >> (12U + i * 9U)) & 0x1ffU;
    }

    return vpn;
}

std::array<uint64_t, 5> Mmu::get_ppn(uint64_t pte)
{
    std::array<uint64_t, 5> ppn = {};

    for (int i = 0; i < get_levels(); i++)
    {
        ppn[i] = (pte >> (10U + i * 9U)) & 0x1ffU;
    }

    return ppn;
}

void Mmu::set_cpu_error(uint64_t address, AccessType access_type)
{
    switch (access_type)
    {
    case Mmu::AccessType::Load:
        cpu.set_exception(exception::Exception::LoadPageFault, address);
        break;
    case Mmu::AccessType::Store:
        cpu.set_exception(exception::Exception::StorePageFault, address);
        break;
    case Mmu::AccessType::Instruction:
        cpu.set_exception(exception::Exception::InstructionPageFault, address);
        break;
    }
}

uint64_t Mmu::translate(uint64_t address, AccessType acces_type)
{
    if (mode == Mode::Bare)
    {
        return address;
    }

    cpu::Mode cpu_mode = cpu.mode;

    if (acces_type != AccessType::Instruction &&
        cpu.cregs.read_bit_mstatus(csr::Mask::MSTATUSBit::MPRV) == 1)
    {
        cpu_mode = cpu.cregs.read_mpp_mode();
    }

    if (cpu_mode == cpu::Mode::Machine)
    {
        return address;
    }

    uint64_t mxr = cpu.cregs.read_bit_mstatus(csr::Mask::MSTATUSBit::MXR);
    uint64_t sum = cpu.cregs.read_bit_mstatus(csr::Mask::MSTATUSBit::SUM);

    auto vpn = get_vpn(address);

    uint64_t levels = get_levels();
    uint64_t pte_size = 8;

    uint64_t a = mppn;
    int64_t i = levels - 1;
    uint64_t pte;
    std::array<uint64_t, 5> ppn;

    bool valid;
    bool read;
    bool write;
    bool execute;
    bool user;
    bool accessed;
    bool dirty;

    for (; i >= 0; i--)
    {
        pte = cpu.bus.load(cpu, a + vpn[i] * pte_size, 64);

        valid = (pte >> Pte::Valid) & 1;
        read = (pte >> Pte::Read) & 1;
        write = (pte >> Pte::Write) & 1;
        execute = (pte >> Pte::Execute) & 1;

        if (!valid || (!read && write))
        {
            set_cpu_error(address, acces_type);
            return 0;
        }

        if (read || execute)
        {
            break;
        }

        a = ((pte >> 10U) & 0xfffffffffffULL) * page_size;
    }

    if (i < 0)
    {
        set_cpu_error(address, acces_type);
        return 0;
    }

    user = (pte >> Pte::User) & 1;
    accessed = (pte >> Pte::Accessed) & 1;
    dirty = (pte >> Pte::Dirty) & 1;

    if ((!read && write && !execute) || (!read && write && execute))
    {
        set_cpu_error(address, acces_type);
        return 0;
    }

    if (user && ((cpu_mode != cpu::Mode::User) && (!sum || acces_type == AccessType::Instruction)))
    {
        set_cpu_error(address, acces_type);
        return 0;
    }

    if (!user && (cpu_mode != cpu::Mode::Supervisor))
    {
        set_cpu_error(address, acces_type);
        return 0;
    }

    switch (acces_type)
    {
    case Mmu::AccessType::Load:
        if (!(read || (execute && mxr)))
        {
            set_cpu_error(address, acces_type);
            return 0;
        }
        break;
    case Mmu::AccessType::Store:
        if (!write)
        {
            set_cpu_error(address, acces_type);
            return 0;
        }
        break;
    case Mmu::AccessType::Instruction:
        if (!execute)
        {
            set_cpu_error(address, acces_type);
            return 0;
        }
    }

    ppn = get_ppn(pte);

    for (int j = 0; j < i; j++)
    {
        if (ppn[j] != 0)
        {
            set_cpu_error(address, acces_type);
            return 0;
        }
    }

    if (!accessed || (acces_type == AccessType::Store && !dirty))
    {
        uint64_t pte_addr = a + vpn[i] * page_size;
        uint64_t cmp = cpu.bus.load(cpu, pte_addr, 64);

        if (pte == cmp)
        {
            pte |= 1 << Pte::Accessed;

            if (acces_type == AccessType::Store)
            {
                pte |= 1 << Pte::Dirty;
            }

            cpu.bus.store(cpu, pte_addr, pte, 64);
        }
    }

    uint64_t phys_addr = address & 0xfffU;

    if (i == 0)
    {
        uint64_t temp = (pte >> 10U) & 0xfffffffffffULL;

        return phys_addr | (temp << 12U);
    }

    for (uint64_t j = 0; j < i; j++)
    {
        phys_addr |= vpn[j] << (12U + (j * 9U));
    }

    for (uint64_t j = i; j < levels; j++)
    {
        phys_addr |= ppn[j] << (12U + (j * 9U));
    }

    return phys_addr;
}

} // namespace mmu
