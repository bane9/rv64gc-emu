#pragma once

#include "bus.hpp"
#include "clint.hpp"
#include "common_def.hpp"
#include "csr.hpp"
#include "gpu.hpp"
#include "interupt.hpp"
#include "misc.hpp"
#include "mmu.hpp"
#include "plic.hpp"
#include "ram.hpp"
#include "virtio.hpp"
#include <array>
#include <iostream>
#include <ostream>
#include <set>

class Decoder;

class Cpu
{
  public:
    Cpu(RamDevice* dram_device, gpu::GpuDevice* gpu_device = nullptr,
        virtio::VirtioBlkDevice* virtio_blk_device = nullptr);

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
    [[noreturn]] void run();
    void loop(std::ostream& debug_stream = std::cout);

  public:
    uint32_t _loop(std::ostream& debug_stream = std::cout);

  public:
    void execute16(Decoder decoder);
    void execute32(Decoder decoder);

  public:
    void set_exception(exception::Exception::ExceptionValue value, uint64_t exc_data = 0);
    void clear_exception();

  public:
    Bus bus;
    mmu::Mmu mmu;

  public:
    std::array<uint64_t, 32> regs = {};
    std::array<Fregister, 32> fregs = {};
    csr::Csr cregs;
    uint64_t pc = 0;
    // For debugging purposes
    uint64_t previous_pc = 0;

  public:
    PlicDevice plic_device;
    ClintDevice clint_device;
    RamDevice* dram_device;
    gpu::GpuDevice* gpu_device;
    virtio::VirtioBlkDevice* virtio_blk_device;

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
    static constexpr auto reg_name_abi_str = std::array<const char*, 32>{
        {"zero", "ra", "sp", "gp", "tp",  "t0",  "t1", "t2", "s0", "s1", "a0",
         "a1",   "a2", "a3", "a4", "a5",  "a6",  "a7", "s2", "s3", "s4", "s5",
         "s6",   "s7", "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"}};

    static constexpr auto freg_name_abi_str = std::array<const char*, 32>{
        {"ft0", "ft1", "ft2", "ft3", "ft4",  "ft5",  "ft6", "ft7", "fs0",  "fs1", "fa0",
         "fa1", "fa2", "fa3", "fa4", "fa5",  "fa6",  "fa7", "fs2", "fs3",  "fs4", "fs5",
         "fs6", "fs7", "fs8", "fs9", "fs10", "fs11", "ft8", "ft9", "ft10", "ft11"}};
};
