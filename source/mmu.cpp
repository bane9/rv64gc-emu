#include "mmu.hpp"
#include "cpu.hpp"
#include "cpu_config.hpp"
#include "helper.hpp"
#include <cassert>
#include <limits>
#include <utility>

namespace mmu
{

Mmu::Mmu(Cpu& cpu) : cpu(cpu)
{
    mode = Mode::Bare;

    flush_tlb();
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

uint64_t Mmu::fetch(uint64_t address, uint64_t length)
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

    this->mppn = ppn << 12ULL;
    this->mode = static_cast<Mode::ModeValue>(mode);

    flush_tlb();
}

uint32_t Mmu::get_levels()
{
    switch (mode)
    {
    case Mode::SV39:
        return 3;
    case Mode::SV48:
        return 4;
    case Mode::SV57:
        return 5;
    default:
        return 0;
    }
}

pn_arr_t Mmu::get_vpn(uint64_t address)
{
    pn_arr_t vpn = {};

    for (int i = 0; i < get_levels(); i++)
    {
        vpn[i] = (address >> (12ULL + i * 9ULL)) & 0x1ffULL;
    }

    return vpn;
}

pn_arr_t Mmu::get_ppn(uint64_t pte)
{
    pn_arr_t ppn;

    for (int i = 0; i < get_levels() - 1; i++)
    {
        ppn[i] = (pte >> (10ULL + i * 9ULL)) & 0x1ffULL;
    }

    switch (mode)
    {
    case Mode::SV39:
        ppn[2] = (pte >> 28) & 0x03ffffffULL;
        break;
    case Mode::SV48:
        ppn[3] = (pte >> 37) & 0x1ffffULL;
        break;
    case Mode::SV57:
        ppn[4] = (pte >> 46) & 0xffULL;
        break;
    default:
        break;
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

    flush_tlb();
}

bool Mmu::fetch_pte(uint64_t address, AccessType acces_type, cpu::Mode cpu_mode, TLBEntry& entry)
{
    pn_arr_t vpn = get_vpn(address);

    uint64_t levels = get_levels();
    constexpr uint64_t pte_size = 8;

    uint64_t a = mppn;
    int64_t i = levels - 1;

    for (; i >= 0; i--)
    {
        entry.pte_addr = a + vpn[i] * pte_size;
        entry.pte = cpu.bus.load(cpu, entry.pte_addr, 64);

        entry.read = (entry.pte >> Pte::Read) & 1;
        entry.write = (entry.pte >> Pte::Write) & 1;
        entry.execute = (entry.pte >> Pte::Execute) & 1;

        bool valid = (entry.pte >> Pte::Valid) & 1;

        if (!valid || (!entry.read && entry.write))
        {
            set_cpu_error(address, acces_type);
            return false;
        }

        if (entry.read || entry.execute)
        {
            break;
        }

        a = ((entry.pte >> 10ULL) & 0xfffffffffffULL) * page_size;
    }

    if (i < 0)
    {
        set_cpu_error(address, acces_type);
        return false;
    }

    pn_arr_t ppn = get_ppn(entry.pte);

    for (int j = 0; j < i; j++)
    {
        if (ppn[j] != 0)
        {
            set_cpu_error(address, acces_type);
            return false;
        }
    }

    entry.user = (entry.pte >> Pte::User) & 1;
    entry.accessed = (entry.pte >> Pte::Accessed) & 1;
    entry.dirty = (entry.pte >> Pte::Dirty) & 1;

    if (i == 0)
    {
        entry.phys_base = ((entry.pte >> 10ULL) & 0x0fffffffffffULL) << 12ULL;
    }
    else
    {
        entry.phys_base = 0;

        for (uint64_t j = 0; j < i; j++)
        {
            entry.phys_base |= vpn[j] << (12ULL + (j * 9ULL));
        }

        for (uint64_t j = i; j < levels; j++)
        {
            entry.phys_base |= ppn[j] << (12ULL + (j * 9ULL));
        }
    }
#if !TLB_COMPLIANT
    bool mxr = cpu.cregs.read_bit_mstatus(csr::Mask::MSTATUSBit::MXR);
    bool sum = cpu.cregs.read_bit_mstatus(csr::Mask::MSTATUSBit::SUM);

    if ((!entry.read && entry.write && !entry.execute) ||
        (!entry.read && entry.write && entry.execute))
    {
        set_cpu_error(address, acces_type);
        return false;
    }

    if (entry.user &&
        ((cpu_mode != cpu::Mode::User) && (!sum || acces_type == AccessType::Instruction)))
    {
        set_cpu_error(address, acces_type);
        return false;
    }

    if (!entry.user && (cpu_mode != cpu::Mode::Supervisor))
    {
        set_cpu_error(address, acces_type);
        return false;
    }

    switch (acces_type)
    {
    case Mmu::AccessType::Load:
        if (!(entry.read || (entry.execute && mxr)))
        {
            set_cpu_error(address, acces_type);
            return false;
        }
        break;
    case Mmu::AccessType::Store:
        if (!entry.write)
        {
            set_cpu_error(address, acces_type);
            return false;
        }
        break;
    case Mmu::AccessType::Instruction:
        if (!entry.execute)
        {
            set_cpu_error(address, acces_type);
            return false;
        }
    }

    if (!entry.accessed || (acces_type == AccessType::Store && !entry.dirty))
    {
        entry.pte |= 1ULL << Pte::Accessed;
        entry.accessed = true;

        if (acces_type == AccessType::Store)
        {
            entry.pte |= 1ULL << Pte::Dirty;
            entry.dirty = true;
        }

        cpu.bus.store(cpu, entry.pte_addr, entry.pte, 64);
    }
#endif

    return true;
}

TLBEntry* Mmu::get_tlb_entry(uint64_t address, AccessType acces_type, cpu::Mode cpu_mode)
{
#if USE_TLB
    uint64_t addr_masked = address & ~0xfffULL;
    uint64_t oldest_tlb_age = 0;
    int oldest_tlb_index = 0;

    for (int i = 0; i < tlb_cache.size(); i++)
    {
        TLBEntry& entry = tlb_cache[i];

        if (addr_masked == entry.virt_base)
        {
            entry.age = 0;

            while (++i < tlb_cache.size())
            {
                ++tlb_cache[i].age;
            }

            return &entry;
        }

        ++entry.age;

        if (entry.age > oldest_tlb_age)
        {
            oldest_tlb_age = entry.age;
            oldest_tlb_index = i;
        }
    }

    TLBEntry& entry = tlb_cache[oldest_tlb_index];

    if (fetch_pte(address, acces_type, cpu_mode, entry))
    {
        entry.virt_base = addr_masked;
        entry.age = 0;
        return &entry;
    }
#else
    TLBEntry& entry = tlb_cache[0];

    if (fetch_pte(address, acces_type, cpu_mode, entry))
    {
        return &entry;
    }
#endif

    return nullptr;
}

void Mmu::flush_tlb()
{
    static constexpr uint32_t tlb_reset_age = std::numeric_limits<uint32_t>::max();

    tlb_cache = {};

    for (TLBEntry& entry : tlb_cache)
    {
        entry.age = tlb_reset_age;
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

    TLBEntry* entry = get_tlb_entry(address, acces_type, cpu_mode);

    if (entry == nullptr) [[unlikely]]
    {
        return 0;
    }

#if TLB_COMPLIANT
    bool mxr = cpu.cregs.read_bit_mstatus(csr::Mask::MSTATUSBit::MXR);
    bool sum = cpu.cregs.read_bit_mstatus(csr::Mask::MSTATUSBit::SUM);

    if ((!entry->read && entry->write && !entry->execute) ||
        (!entry->read && entry->write && entry->execute))
    {
        set_cpu_error(address, acces_type);
        return false;
    }

    if (entry->user &&
        ((cpu_mode != cpu::Mode::User) && (!sum || acces_type == AccessType::Instruction)))
    {
        set_cpu_error(address, acces_type);
        return false;
    }

    if (!entry->user && (cpu_mode != cpu::Mode::Supervisor))
    {
        set_cpu_error(address, acces_type);
        return false;
    }

    switch (acces_type)
    {
    case Mmu::AccessType::Load:
        if (!(entry->read || (entry->execute && mxr)))
        {
            set_cpu_error(address, acces_type);
            return false;
        }
        break;
    case Mmu::AccessType::Store:
        if (!entry->write)
        {
            set_cpu_error(address, acces_type);
            return false;
        }
        break;
    case Mmu::AccessType::Instruction:
        if (!entry->execute)
        {
            set_cpu_error(address, acces_type);
            return false;
        }
    }

    if (!entry->accessed || (acces_type == AccessType::Store && !entry->dirty))
    {
        entry->pte |= 1ULL << Pte::Accessed;
        entry->accessed = true;

        if (acces_type == AccessType::Store)
        {
            entry->pte |= 1ULL << Pte::Dirty;
            entry->dirty = true;
        }

        cpu.bus.store(cpu, entry->pte_addr, entry->pte, 64);
    }
#endif
    return entry->phys_base | (address & 0xfffULL);
}

} // namespace mmu
