#pragma once
#include <array>
#include <cstdint>

#define NONE_VAL (~0U)

class Cpu;

namespace interrupt
{
struct Interrupt
{
    enum InterruptValue : uint64_t
    {
        UserSoftware = 0,
        SupervisorSoftware = 1,
        MachineSoftware = 3,
        UserTimer = 4,
        SupervisorTimer = 5,
        MachineTimer = 7,
        UserExternal = 8,
        SupervisorExternal = 9,
        MachineExternal = 11,

        None = NONE_VAL
    };

    static const char* get_interrupt_str(interrupt::Interrupt::InterruptValue value);
};

Interrupt::InterruptValue get_pending_interrupt(Cpu& cpu);
void process(Cpu& cpu, Interrupt::InterruptValue int_val);

} // namespace interrupt

namespace exception
{
struct Exception
{
    enum ExceptionValue : uint64_t
    {
        InstructionAddressMisaligned = 0,
        InstructionAccessFault = 1,
        IllegalInstruction = 2,
        Breakpoint = 3,
        LoadAddressMisaligned = 4,
        LoadAccessFault = 5,
        StoreAddressMisaligned = 6,
        StoreAccessFault = 7,
        ECallUMode = 8,
        ECallSMode = 9,
        ECallMMode = 11,
        InstructionPageFault = 12,
        LoadPageFault = 13,
        StorePageFault = 15,

        None = NONE_VAL
    };

    static const char* get_exception_str(ExceptionValue value);
};

void process(Cpu& cpu);
uint64_t get_pc(Cpu& cpu);
uint64_t get_value(Cpu& cpu);

} // namespace exception