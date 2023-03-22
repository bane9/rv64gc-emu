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

    static constexpr auto interrupt_str = std::array<const char*, MachineExternal + 1>{{
        [UserSoftware] = "UserSoftware",
        [SupervisorSoftware] = "SupervisorSoftware",
        [MachineSoftware] = "MachineSoftware",
        [UserTimer] = "UserTimer",
        [SupervisorTimer] = "SupervisorTimer",
        [MachineTimer] = "MachineTimer",
        [UserExternal] = "UserExternal",
        [SupervisorExternal] = "SupervisorExternal",
        [MachineExternal] = "MachineExternal",
    }};

    static const char* get_interrupt_str(interrupt::Interrupt::InterruptValue value);
};

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

    static constexpr auto exception_str = std::array<const char*, StorePageFault + 1>{{
        [InstructionAddressMisaligned] = "InstructionAddressMisaligned",
        [InstructionAccessFault] = "InstructionAccessFault",
        [IllegalInstruction] = "IllegalInstruction",
        [Breakpoint] = "Breakpoint",
        [LoadAddressMisaligned] = "LoadAddressMisaligned",
        [LoadAccessFault] = "LoadAccessFault",
        [StoreAddressMisaligned] = "StoreAddressMisaligned",
        [StoreAccessFault] = "StoreAccessFault",
        [ECallUMode] = "ECallUMode",
        [ECallSMode] = "ECallSMode",
        [ECallMMode] = "ECallMMode",
        [InstructionPageFault] = "InstructionPageFault",
        [LoadPageFault] = "LoadPageFault",
        [StorePageFault] = "StorePageFault",
    }};

    static const char* get_exception_str(ExceptionValue value);
};

void process(Cpu& cpu);
uint64_t get_pc(Cpu& cpu);
uint64_t get_value(Cpu& cpu);

} // namespace exception