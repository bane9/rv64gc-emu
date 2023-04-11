#include "cpu.hpp"
#include "interupt.hpp"
#include <fmt/core.h>

void interrupt::process(Cpu& cpu, Interrupt::InterruptValue int_val)
{
    if (int_val == Interrupt::None)
    {
        return;
    }

    cpu.sleep = false;

    uint64_t pc = cpu.pc - 4;
    cpu::Mode mode = cpu.mode;

    bool mideleg_flag = (cpu.cregs.load(csr::Address::MIDELEG) >> int_val) & 1;
    std::cout << fmt::format(
        "int = {}, mideleg={}, mode={}, mode_check={}, happened at pc=0x{:0>8x}\n", (int)int_val,
        mideleg_flag, (int)cpu.mode, mode <= cpu::Mode::Supervisor, cpu.pc);

    if ((mode == cpu::Mode::User || mode == cpu::Mode::Supervisor) && mideleg_flag)
    {
        cpu.mode = cpu::Mode::Supervisor;

        uint64_t stvec_val = cpu.cregs.load(csr::Address::STVEC);
        uint64_t vt_offset = 0;

        if (stvec_val & 1)
        {
            vt_offset = int_val * 4;
        }

        cpu.pc = (stvec_val & ~3ULL) + vt_offset;

        cpu.cregs.store(csr::Address::SEPC, pc & ~1ULL);

        cpu.cregs.store(csr::Address::SCAUSE, (1ULL << 63ULL) | int_val);

        cpu.cregs.store(csr::Address::STVAL, 0);

        cpu.cregs.write_bit_sstatus(csr::Mask::SSTATUSBit::SPIE,
                                    cpu.cregs.read_bit_sstatus(csr::Mask::SSTATUSBit::SIE));

        cpu.cregs.write_bit_sstatus(csr::Mask::SSTATUSBit::SIE, 0);

        cpu.cregs.write_bit_sstatus(csr::Mask::SSTATUSBit::SPP, mode);
    }
    else
    {
        cpu.mode = cpu::Mode::Machine;

        uint64_t mtvec_val = cpu.cregs.load(csr::Address::MTVEC);
        uint64_t vt_offset = 0;

        if (mtvec_val & 1)
        {
            vt_offset = int_val * 4;
        }

        cpu.pc = (mtvec_val & ~3ULL) + vt_offset;

        cpu.cregs.store(csr::Address::MEPC, pc & ~1ULL);

        cpu.cregs.store(csr::Address::MCAUSE, (1ULL << 63ULL) | int_val);

        cpu.cregs.store(csr::Address::MTVAL, 0);

        cpu.cregs.write_bit_mstatus(csr::Mask::MSTATUSBit::MPIE,
                                    cpu.cregs.read_bit_mstatus(csr::Mask::MSTATUSBit::MIE));

        cpu.cregs.write_bit_mstatus(csr::Mask::MSTATUSBit::MIE, 0);

        cpu.cregs.write_mpp_mode(mode);
    }
}

void exception::process(Cpu& cpu)
{
    cpu.sleep = false;

    uint64_t pc = cpu.pc;
    cpu::Mode mode = cpu.mode;

    bool medeleg_flag = (cpu.cregs.load(csr::Address::MEDELEG) >> cpu.exc_val) & 1;
    std::cout << fmt::format(
        "exception = {}, medeleg={}, mode={}, mode_check={}, happened at pc=0x{:0>8x}\n",
        (int)cpu.exc_val, medeleg_flag, (int)cpu.mode, mode <= cpu::Mode::Supervisor, cpu.pc);

    if ((mode == cpu::Mode::User || mode == cpu::Mode::Supervisor) && medeleg_flag)
    {
        cpu.mode = cpu::Mode::Supervisor;

        uint64_t stvec_val = cpu.cregs.load(csr::Address::STVEC);

        cpu.pc = stvec_val & ~3ULL;

        cpu.cregs.store(csr::Address::SEPC, pc & ~1ULL);

        cpu.cregs.store(csr::Address::SCAUSE, cpu.exc_val);

        cpu.cregs.store(csr::Address::STVAL, get_value(cpu));

        cpu.cregs.write_bit_sstatus(csr::Mask::SSTATUSBit::SPIE,
                                    cpu.cregs.read_bit_sstatus(csr::Mask::SSTATUSBit::SIE));

        cpu.cregs.write_bit_sstatus(csr::Mask::SSTATUSBit::SIE, 0);

        cpu.cregs.write_bit_sstatus(csr::Mask::SSTATUSBit::SPP, mode);
    }
    else
    {
        cpu.mode = cpu::Mode::Machine;

        uint64_t mtvec_val = cpu.cregs.load(csr::Address::MTVEC);

        cpu.pc = mtvec_val & ~3ULL;

        cpu.cregs.store(csr::Address::MEPC, pc & ~1ULL);

        cpu.cregs.store(csr::Address::MCAUSE, cpu.exc_val);

        cpu.cregs.store(csr::Address::MTVAL, get_value(cpu));

        cpu.cregs.write_bit_mstatus(csr::Mask::MSTATUSBit::MPIE,
                                    cpu.cregs.read_bit_mstatus(csr::Mask::MSTATUSBit::MIE));

        cpu.cregs.write_bit_mstatus(csr::Mask::MSTATUSBit::MIE, 0);

        cpu.cregs.write_mpp_mode(mode);
    }
}

uint64_t exception::get_value(Cpu& cpu)
{
    switch (cpu.exc_val)
    {
    case Exception::InstructionAddressMisaligned:
    case Exception::InstructionAccessFault:
    case Exception::Breakpoint:
    case Exception::LoadAddressMisaligned:
    case Exception::LoadAccessFault:
    case Exception::StoreAddressMisaligned:
    case Exception::StoreAccessFault:
    case Exception::InstructionPageFault:
    case Exception::LoadPageFault:
    case Exception::StorePageFault:
        return cpu.pc;
    case Exception::IllegalInstruction:
        return cpu.exc_data;
    default:
        return 0;
    }
}

const char* exception::Exception::get_exception_str(ExceptionValue value)
{
    switch (value)
    {
    case InstructionAddressMisaligned:
        return "InstructionAddressMisaligned";
    case InstructionAccessFault:
        return "InstructionAccessFault";
    case IllegalInstruction:
        return "IllegalInstruction";
    case Breakpoint:
        return "Breakpoint";
    case LoadAddressMisaligned:
        return "LoadAddressMisaligned";
    case LoadAccessFault:
        return "LoadAccessFault";
    case StoreAddressMisaligned:
        return "StoreAddressMisaligned";
    case StoreAccessFault:
        return "StoreAccessFault";
    case ECallUMode:
        return "ECallUMode";
    case ECallSMode:
        return "ECallSMode";
    case ECallMMode:
        return "ECallMMode";
    case InstructionPageFault:
        return "InstructionPageFault";
    case LoadPageFault:
        return "LoadPageFault";
    case StorePageFault:
        return "StorePageFault";
    default:
        return "Unknown exception";
    }
}

const char* interrupt::Interrupt::get_interrupt_str(interrupt::Interrupt::InterruptValue value)
{
    switch (value)
    {
    case UserSoftware:
        return "UserSoftware";
    case SupervisorSoftware:
        return "SupervisorSoftware";
    case MachineSoftware:
        return "MachineSoftware";
    case UserTimer:
        return "UserTimer";
    case SupervisorTimer:
        return "SupervisorTimer";
    case MachineTimer:
        return "MachineTimer";
    case UserExternal:
        return "UserExternal";
    case SupervisorExternal:
        return "SupervisorExternal";
    case MachineExternal:
        return "MachineExternal";
    default:
        return "Unknown interrupt";
    }
}
