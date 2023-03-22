#pragma once

#include "bus.hpp"
#include "common_def.hpp"
#include "csr.hpp"
#include "interupt.hpp"
#include "misc.hpp"
#include "mmu.hpp"
#include <array>
#include <iostream>
#include <ostream>
#include <set>

class Decoder;

class Cpu
{
  public:
    Cpu(uint64_t dram_begin, uint64_t dram_end);

    void dump_registers(std::ostream& stream);

    void dump_bus_devices(std::ostream& stream);

  public:
    enum reg_name : uint64_t
    {
        x0 = 0,
        x1 = 1,
        x2 = 2,
        x3 = 3,
        x4 = 4,
        x5 = 5,
        x6 = 6,
        x7 = 7,
        x8 = 8,
        x9 = 9,
        x10 = 10,
        x11 = 11,
        x12 = 12,
        x13 = 13,
        x14 = 14,
        x15 = 15,
        x16 = 16,
        x17 = 17,
        x18 = 18,
        x19 = 19,
        x20 = 20,
        x21 = 21,
        x22 = 22,
        x23 = 23,
        x24 = 24,
        x25 = 25,
        x26 = 26,
        x27 = 27,
        x28 = 28,
        x29 = 29,
        x30 = 30,
        x31 = 31
    };

    enum reg_abi_name : uint64_t
    {
        zero = 0,
        ra = 1,
        sp = 2,
        gp = 3,
        tp = 4,
        t0 = 5,
        t1 = 6,
        t2 = 7,
        s0 = 8,
        fp = s0,
        s1 = 9,
        a0 = 10,
        a1 = 11,
        a2 = 12,
        a3 = 13,
        a4 = 14,
        a5 = 15,
        a6 = 16,
        a7 = 17,
        s2 = 18,
        s3 = 19,
        s4 = 20,
        s5 = 21,
        s6 = 22,
        s7 = 23,
        s8 = 24,
        s9 = 25,
        s10 = 26,
        s11 = 27,
        t3 = 28,
        t4 = 29,
        t5 = 30,
        t6 = 31
    };

    enum freg_abi_name : uint64_t
    {
        ft0 = 0,
        ft1 = 1,
        ft2 = 2,
        ft3 = 3,
        ft4 = 4,
        ft5 = 5,
        ft6 = 6,
        ft7 = 7,
        fs0 = 8,
        fs1 = 9,
        fa0 = 10,
        fa1 = 11,
        fa2 = 12,
        fa3 = 13,
        fa4 = 14,
        fa5 = 15,
        fa6 = 16,
        fa7 = 17,
        fs2 = 18,
        fs3 = 19,
        fs4 = 20,
        fs5 = 21,
        fs6 = 22,
        fs7 = 23,
        fs8 = 24,
        fs9 = 25,
        fs10 = 26,
        fs11 = 27,
        ft8 = 28,
        ft9 = 29,
        ft10 = 30,
        ft11 = 31
    };

  public:
    void run();
    void loop(std::ostream& debug_stream = std::cout);

  public:
    void _loop(std::ostream& debug_stream = std::cout);

  public:
    void execute16(Decoder decoder);
    void execute32(Decoder decoder);

  public:
    interrupt::Interrupt::InterruptValue get_pending_interrupt();
    void handle_irq(std::ostream& debug_stream);

  public:
    void set_exception(exception::Exception::ExceptionValue value, uint64_t exc_data = 0);
    void clear_exception();

  public:
    std::array<uint64_t, 32> regs = {};
    std::array<Fregister, 32> fregs = {};
    csr::Csr cregs;
    uint64_t pc = 0;
    Bus bus;

  public:
    mmu::Mmu mmu;

  public:
    cpu::Mode mode;

  public:
    exception::Exception::ExceptionValue exc_val = exception::Exception::None;
    uint64_t exc_data = 0;

  public:
    bool sleep = false;

  public:
    std::set<uint64_t> reservations;

  public:
    static constexpr auto reg_name_abi_str = std::array<const char*, 32>{{
        [reg_abi_name::zero] = "zero", [reg_abi_name::ra] = "ra", [reg_abi_name::sp] = "sp",
        [reg_abi_name::gp] = "gp",     [reg_abi_name::tp] = "tp", [reg_abi_name::t0] = "t0",
        [reg_abi_name::t1] = "t1",     [reg_abi_name::t2] = "t2", [reg_abi_name::s0] = "s0",
        [reg_abi_name::s1] = "s1",     [reg_abi_name::a0] = "a0", [reg_abi_name::a1] = "a1",
        [reg_abi_name::a2] = "a2",     [reg_abi_name::a3] = "a3", [reg_abi_name::a4] = "a4",
        [reg_abi_name::a5] = "a5",     [reg_abi_name::a6] = "a6", [reg_abi_name::a7] = "a7",
        [reg_abi_name::s2] = "s2",     [reg_abi_name::s3] = "s3", [reg_abi_name::s4] = "s4",
        [reg_abi_name::s5] = "s5",     [reg_abi_name::s6] = "s6", [reg_abi_name::s7] = "s7",
        [reg_abi_name::s8] = "s8",     [reg_abi_name::s9] = "s9", [reg_abi_name::s10] = "s10",
        [reg_abi_name::s11] = "s11",   [reg_abi_name::t3] = "t3", [reg_abi_name::t4] = "t4",
        [reg_abi_name::t5] = "t5",     [reg_abi_name::t6] = "t6",
    }};

    static constexpr auto freg_name_abi_str = std::array<const char*, 32>{
        {[freg_abi_name::ft0] = "ft0",   [freg_abi_name::ft1] = "ft1",
         [freg_abi_name::ft2] = "ft2",   [freg_abi_name::ft3] = "ft3",
         [freg_abi_name::ft4] = "ft4",   [freg_abi_name::ft5] = "ft5",
         [freg_abi_name::ft6] = "ft6",   [freg_abi_name::ft7] = "ft7",
         [freg_abi_name::fs0] = "fs0",   [freg_abi_name::fs1] = "fs1",
         [freg_abi_name::fa0] = "fa0",   [freg_abi_name::fa1] = "fa1",
         [freg_abi_name::fa2] = "fa2",   [freg_abi_name::fa3] = "fa3",
         [freg_abi_name::fa4] = "fa4",   [freg_abi_name::fa5] = "fa5",
         [freg_abi_name::fa6] = "fa6",   [freg_abi_name::fa7] = "fa7",
         [freg_abi_name::fs2] = "fs2",   [freg_abi_name::fs3] = "fs3",
         [freg_abi_name::fs4] = "fs4",   [freg_abi_name::fs5] = "fs5",
         [freg_abi_name::fs6] = "fs6",   [freg_abi_name::fs7] = "fs7",
         [freg_abi_name::fs8] = "fs8",   [freg_abi_name::fs9] = "fs9",
         [freg_abi_name::fs10] = "fs10", [freg_abi_name::fs11] = "fs11",
         [freg_abi_name::ft8] = "ft8",   [freg_abi_name::ft9] = "ft9",
         [freg_abi_name::ft10] = "ft10", [freg_abi_name::ft11] = "ft11"}};
};
