#include "cpu.hpp"
#include "interupt.hpp"

void interrupt::process(Cpu& cpu, Interrupt::InterruptValue int_val)
{
    if (int_val == Interrupt::None)
    {
        return;
    }

    cpu.sleep = false;

    uint64_t pc = cpu.pc;
    cpu::Mode mode = cpu.mode;

    bool mideleg_flag = (cpu.cregs.load(csr::Address::MIDELEG) >> int_val) & 1;

    if (mode <= cpu::Mode::Supervisor && mideleg_flag && int_val != Interrupt::MachineTimer)
    {
        cpu.mode = cpu::Mode::Supervisor;

        uint64_t stvec_val = cpu.cregs.load(csr::Address::STVEC);
        uint64_t vt_offset = 0;

        if (stvec_val & 1)
        {
            vt_offset = int_val * 4;
        }

        cpu.pc = (stvec_val & ~1U) + vt_offset;

        cpu.cregs.store(csr::Address::SEPC, pc & ~1U);

        cpu.cregs.store(csr::Address::SCAUSE, (1ULL << 63ULL) | int_val);

        cpu.cregs.store(csr::Address::STVAL, 0);

        cpu.cregs.write_bit_sstatus(csr::Mask::SSTATUSBit::SPIE,
                                    cpu.cregs.read_bit_sstatus(csr::Mask::SSTATUSBit::SIE));

        cpu.cregs.write_bit_sstatus(csr::Mask::SSTATUSBit::SIE, 0);

        cpu.cregs.write_bit_sstatus(csr::Mask::SSTATUSBit::SPP, mode != cpu::Mode::User);
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

        cpu.pc = (mtvec_val & ~1U) + vt_offset;

        cpu.cregs.store(csr::Address::MEPC, pc & ~1U);

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

    uint64_t pc = get_pc(cpu);
    cpu::Mode mode = cpu.mode;

    bool mideleg_flag = (cpu.cregs.load(csr::Address::MIDELEG) >> cpu.exc_val) & 1;

    if (mode <= cpu::Mode::Supervisor && mideleg_flag)
    {
        cpu.mode = cpu::Mode::Supervisor;

        uint64_t stvec_val = cpu.cregs.load(csr::Address::STVEC);

        cpu.pc = stvec_val & ~1U;

        cpu.cregs.store(csr::Address::SEPC, pc & ~1U);

        cpu.cregs.store(csr::Address::SCAUSE, cpu.exc_val);

        cpu.cregs.store(csr::Address::STVAL, get_value(cpu));

        cpu.cregs.write_bit_sstatus(csr::Mask::SSTATUSBit::SPIE,
                                    cpu.cregs.read_bit_sstatus(csr::Mask::SSTATUSBit::SIE));

        cpu.cregs.write_bit_sstatus(csr::Mask::SSTATUSBit::SIE, 0);

        cpu.cregs.write_bit_sstatus(csr::Mask::SSTATUSBit::SPP, mode != cpu::Mode::User);
    }
    else
    {
        cpu.mode = cpu::Mode::Machine;

        uint64_t mtvec_val = cpu.cregs.load(csr::Address::MTVEC);

        cpu.pc = mtvec_val & ~1U;

        cpu.cregs.store(csr::Address::MEPC, pc & ~1U);

        cpu.cregs.store(csr::Address::MCAUSE, cpu.exc_val);

        cpu.cregs.store(csr::Address::MTVAL, get_value(cpu));

        cpu.cregs.write_bit_mstatus(csr::Mask::MSTATUSBit::MPIE,
                                    cpu.cregs.read_bit_mstatus(csr::Mask::MSTATUSBit::MIE));

        cpu.cregs.write_bit_mstatus(csr::Mask::MSTATUSBit::MIE, 0);

        cpu.cregs.write_mpp_mode(mode);
    }
}

uint64_t exception::get_pc(Cpu& cpu)
{
    switch (cpu.exc_val)
    {
    case Exception::Breakpoint:
    case Exception::InstructionPageFault:
    case Exception::LoadPageFault:
    case Exception::StorePageFault:
    case Exception::IllegalInstruction:
        return cpu.pc - 4;
    default:
        return cpu.pc;
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
        return cpu.pc;
    case Exception::InstructionPageFault:
    case Exception::LoadPageFault:
    case Exception::StorePageFault:
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
    case InstructionAccessFault:
    case IllegalInstruction:
    case Breakpoint:
    case LoadAddressMisaligned:
    case LoadAccessFault:
    case StoreAddressMisaligned:
    case StoreAccessFault:
    case ECallUMode:
    case ECallSMode:
    case ECallMMode:
    case InstructionPageFault:
    case LoadPageFault:
    case StorePageFault:
        return exception_str[value];
    default:
        return "Unknown exception";
    }
}

const char* interrupt::Interrupt::get_interrupt_str(interrupt::Interrupt::InterruptValue value)
{
    switch (value)
    {
    case UserSoftware:
    case SupervisorSoftware:
    case MachineSoftware:
    case UserTimer:
    case SupervisorTimer:
    case MachineTimer:
    case UserExternal:
    case SupervisorExternal:
    case MachineExternal:
        return interrupt_str[value];
    default:
        return "Unknown interrupt";
    }
}
