#pragma once

#include "bus.hpp"
#include <array>
#include <cstdint>

class Cpu;

namespace mmu
{

constexpr uint64_t page_size = 4096;

struct Mode
{
    enum ModeValue : uint64_t
    {
        Bare = 0x00,
        SV39 = 0x08,
        SV48 = 0x09,
        SV57 = 0x0A,
    };
};

struct Pte
{
    enum PteValue : uint64_t
    {
        Valid = 0,
        Read = 1,
        Write = 2,
        Execute = 3,
        User = 4,
        Global = 5,
        Accessed = 6,
        Dirty = 7,
    };
};

class Mmu
{
  public:
    Mmu(Cpu& cpu);

    uint64_t load(uint64_t address, uint64_t length);
    uint64_t fetch(uint64_t address, uint64_t length = 32);
    void store(uint64_t address, uint64_t value, uint64_t length);

  public:
    enum class AccessType
    {
        Load,
        Store,
        Instruction
    };

  public:
    void update();
    uint64_t translate(uint64_t address, AccessType acces_type);

  public:
    uint32_t get_levels();
    std::array<uint64_t, 5> get_vpn(uint64_t address);
    std::array<uint64_t, 5> get_ppn(uint64_t pte);

  public:
    void set_cpu_error(uint64_t address, AccessType access_type);

  public:
    Mode::ModeValue mode;
    uint32_t mppn;

  public:
    Cpu& cpu;
};
} // namespace mmu