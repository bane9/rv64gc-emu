#pragma once
#include "common_def.hpp"
#include <array>
#include <cstdint>

namespace csr
{
struct Address
{
    enum AddressValues : uint64_t
    {
        USTATUS = 0x0,

        FFLAGS = 0x1,
        FRM = 0x2,
        FCSR = 0x3,

        UVEC = 0x5,
        UEPC = 0x41,
        UCAUSE = 0x42,
        UTVAL = 0x43,

        SSTATUS = 0x100,
        SEDELEG = 0x102,
        SIDELEG = 0x103,
        SIE = 0x104,
        STVEC = 0x105,

        SSCRATCH = 0x140,
        SEPC = 0x141,
        SCAUSE = 0x142,
        STVAL = 0x143,
        SIP = 0x144,

        SATP = 0x180,

        MSTATUS = 0x300,
        MISA = 0x301,
        MEDELEG = 0x302,
        MIDELEG = 0x303,
        MIE = 0x304,
        MTVEC = 0x305,
        MCOUNTEREN = 0x306,

        MSCRATCH = 0x340,
        MEPC = 0x341,
        MCAUSE = 0x342,
        MTVAL = 0x343,
        MIP = 0x344,

        CYCLE = 0xc00,
        TIME = 0xc01,
        TIMEMS = 0xc10,

        TDATA1 = 0x7a1,

        MVENDORID = 0xf11,
        MARCHID = 0xf12,
        MIMPID = 0xf13,
        MHARTID = 0xf14,
    };
};

struct Mask
{
    enum SSTATUS_MASK : uint64_t
    {
        SIE = 1U << 1U,
        SPIE = 1U << 5U,
        UBE = 1U << 6U,
        SPP = 1U << 8U,
        FS = 0x6000U,
        XS = 0x18000U,
        SUM = 1U << 18U,
        MXR = 1U << 19U,
        UXL = 0x300000000ULL,
        SD = 1ULL << 63ULL,

        SSTATUS = SIE | SPIE | UBE | SPP | FS | XS | SUM | MXR | UXL | SD,
    };

    enum class SSTATUSBit : uint64_t
    {
        SIE = 0,
        SPIE = 5,
        SPP = 8
    };

    enum class MSTATUSBit : uint64_t
    {
        MIE = 3,
        MPIE = 7,
        MPP = 12,
        MPRV = 17,
        SUM = 18,
        MXR = 19,
        TVM = 20,
        TSR = 22,
    };

    enum Bit : uint64_t
    {
        SSIP_BIT = 1ULL,
        MSIP_BIT = 3ULL,
        STIP_BIT = 5ULL,
        MTIP_BIT = 7ULL,
        SEIP_BIT = 9ULL,
        MEIP_BIT = 11ULL,

        SSIP = 1ULL << SSIP_BIT,
        MSIP = 1ULL << MSIP_BIT,
        STIP = 1ULL << STIP_BIT,
        MTIP = 1ULL << MTIP_BIT,
        SEIP = 1ULL << SEIP_BIT,
        MEIP = 1ULL << MEIP_BIT,
    };
};

struct Misa
{
    enum Extension : uint64_t
    {
        A_EXT = 1U << 0U,
        C_EXT = 1U << 2U,
        D_EXT = 1U << 3U,
        RV32E = 1U << 4U,
        F_EXT = 1U << 5U,
        HYPERVISOR = 1U << 7U,
        RV32I_64I_128I = 1U << 8U,
        M_EXT = 1U << 12U,
        N_EXT = 1U << 13U,
        QUAD_EXT = 1U << 16U,
        SUPERVISOR = 1U << 18U,
        USER = 1U << 20U,
        NON_STD_PRESENT = 1U << 22U,

        XLEN_32 = 1U << 31U,
        XLEN_64 = 2ULL << 62U
    };
};

struct FExcept
{
    enum FExceptVal : uint64_t
    {
        Inexact = 1 << 0,
        Undeflow = 1 << 1,
        Overflow = 1 << 2,
        DivByZero = 1 << 3,
        Invalid = 1 << 4,

        Mask = Inexact | Undeflow | Overflow | DivByZero | Invalid
    };
};

struct FS
{
    enum FSVal : uint64_t
    {
        Off = 0,
        Initial = 1,
        Clean = 2,
        Dirty = 3,
    };
};

class Csr
{
  public:
    Csr();

    uint64_t load(uint64_t address);
    void store(uint64_t address, uint64_t value);

    uint64_t read_bit(uint64_t address, uint64_t offset);
    uint64_t read_bits(uint64_t address, uint64_t upper_offset, uint64_t lower_offset);

    void write_bit(uint64_t address, uint64_t offset, uint64_t value);
    void write_bits(uint64_t address, uint64_t upper_offset, uint64_t lower_offset, uint64_t value);

    uint64_t read_bit_mstatus(Mask::MSTATUSBit bit);
    void write_bit_mstatus(Mask::MSTATUSBit bit, uint64_t value);

    cpu::Mode read_mpp_mode();
    void write_mpp_mode(cpu::Mode mode);

    uint64_t read_bit_sstatus(Mask::SSTATUSBit bit);
    void write_bit_sstatus(Mask::SSTATUSBit bit, uint64_t value);

    void set_fpu_exception(FExcept::FExceptVal exception);
    void clear_fpu_exceptions();

    void set_fpu_rm(FPURoundigMode::Mode round_mode);
    void clear_fpu_rm();

    void set_fs(FS::FSVal value);
    FS::FSVal get_fs();
    bool is_fpu_enabled();

  public:
    std::array<uint64_t, 4096> regs = {};
};
} // namespace csr