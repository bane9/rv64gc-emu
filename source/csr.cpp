#include "csr.hpp"
#include "helper.hpp"

namespace csr
{
Csr::Csr()
{
    regs[Address::MISA] = Misa::XLEN_64 | Misa::A_EXT | Misa::C_EXT | Misa::F_EXT | Misa::D_EXT |
                          Misa::RV32I_64I_128I | Misa::M_EXT | Misa::SUPERVISOR | Misa::USER;
}

uint64_t Csr::load(uint64_t address)
{
    switch (address)
    {
    case Address::SSTATUS:
        return regs[Address::MSTATUS] & Mask::SSTATUS;
    case Address::SIE:
        return regs[Address::MIE] & regs[Address::MIDELEG];
    case Address::SIP:
        return regs[Address::MIP] & regs[Address::MIDELEG];
    case Address::MSTATUS:
        return regs[Address::MSTATUS] | 0x200000000ULL;
    default:
        return regs[address];
    }
}

void Csr::store(uint64_t address, uint64_t value)
{
    switch (address)
    {
    case Address::SSTATUS: {
        uint64_t val = (regs[Address::MSTATUS] & ~Mask::SSTATUS) | (value & Mask::SSTATUS);
        regs[Address::MSTATUS] = val;
        break;
    }
    case Address::SIE: {
        uint64_t val =
            (regs[Address::MIE] & ~regs[Address::MIDELEG]) | (value & regs[Address::MIDELEG]);
        regs[Address::MIE] = val;
        break;
    }
    case Address::SIP: {
        uint64_t mask = regs[Address::MIDELEG] & Mask::SSIP;
        uint64_t val = (regs[Address::MIP] & ~mask) | (value & mask);
        regs[Address::MIP] = val;
        break;
    }
    default:
        regs[address] = value;
    }
}

uint64_t Csr::read_bit(uint64_t address, uint64_t offset)
{
    return helper::read_bit(regs[address], offset);
}

uint64_t Csr::read_bits(uint64_t address, uint64_t upper_offset, uint64_t lower_offset)
{
    return helper::read_bits(regs[address], upper_offset, lower_offset);
}

void Csr::write_bit(uint64_t address, uint64_t offset, uint64_t value)
{
    regs[address] = helper::write_bit(regs[address], offset, value);
}

void Csr::write_bits(uint64_t address, uint64_t upper_offset, uint64_t lower_offset, uint64_t value)
{
    regs[address] = helper::write_bits(regs[address], upper_offset, lower_offset, value);
}

uint64_t Csr::read_bit_mstatus(Mask::MSTATUSBit bit)
{
    return read_bit(Address::MSTATUS, static_cast<uint64_t>(bit));
}

void Csr::write_bit_mstatus(Mask::MSTATUSBit bit, uint64_t value)
{
    write_bit(Address::MSTATUS, static_cast<uint64_t>(bit), value);
}

void Csr::write_mpp_mode(cpu::Mode mode)
{
    write_bits(Address::MSTATUS, 12, 11, mode);
}

cpu::Mode Csr::read_mpp_mode()
{
    uint64_t mpp = read_bits(Address::MSTATUS, 12, 11);

    switch (mpp)
    {
    case cpu::Mode::User:
    case cpu::Mode::Supervisor:
    case cpu::Mode::Machine:
        return static_cast<cpu::Mode>(mpp);
    default:
        return cpu::Mode::Invalid;
    }
}

uint64_t Csr::read_bit_sstatus(Mask::SSTATUSBit bit)
{
    return read_bit(Address::SSTATUS, static_cast<uint64_t>(bit));
}

void Csr::write_bit_sstatus(Mask::SSTATUSBit bit, uint64_t value)
{
    write_bit(Address::SSTATUS, static_cast<uint64_t>(bit), value);
}

void Csr::set_fpu_exception(FExcept::FExceptVal exception)
{
    regs[Address::FFLAGS] |= exception;
}

void Csr::clear_fpu_exceptions()
{
    regs[Address::FFLAGS] &= ~FExcept::Mask;
}

void Csr::set_fpu_rm(FPURoundigMode::Mode round_mode)
{
    regs[Address::FRM] |= round_mode;
}

void Csr::clear_fpu_rm()
{
    regs[Address::FRM] &= ~FPURoundigMode::Mode::Mask;
}

void Csr::set_fs(FS::FSVal value)
{
    write_bits(Address::MSTATUS, 14, 13, value);
}

FS::FSVal Csr::get_fs()
{
    return static_cast<FS::FSVal>(read_bits(Address::MSTATUS, 14, 13));
}

bool Csr::is_fpu_enabled()
{
    return get_fs() != FS::Off;
}
} // namespace csr