#pragma once

#include "bus.hpp"
#include "cpu_config.hpp"
#include <array>

class PlicDevice : public BusDevice
{
  public:
    PlicDevice() = default;
    virtual ~PlicDevice() = default;

    uint64_t load(Bus& bus, uint64_t address, uint64_t length) override;
    void store(Bus& bus, uint64_t address, uint64_t value, uint64_t length) override;

  public:
    void update_pending(uint64_t irq);
    void clear_pending(uint64_t irq);
    void update_claim(uint64_t irq);
    bool is_enabled(uint64_t context, uint64_t irq);

  public:
    uint64_t get_base_address() const override;
    uint64_t get_end_address() const override;

    void dump(std::ostream& stream) const override;

    std::string_view get_peripheral_name() const override;

  public:
    std::array<uint32_t, 1024> prioprity = {};
    std::array<uint32_t, 32> pending = {};
    std::array<uint32_t, 64> enable = {};
    std::array<uint32_t, 2> treshold = {};
    std::array<uint32_t, 2> claim = {};

  public:
    static constexpr uint64_t base_addr = 0xC000000ULL;

    static constexpr uint64_t source_prio_addr = base_addr;
    static constexpr uint64_t source_prio_end_addr = base_addr + 0xfffULL;

    static constexpr uint64_t pending_addr = base_addr + 0x1000ULL;
    static constexpr uint64_t pending_end_addr = base_addr + 0x107fULL;

    static constexpr uint64_t enable_addr = base_addr + 0x2000ULL;
    static constexpr uint64_t enable_end_addr = base_addr + 0x20ffULL;

    static constexpr uint64_t treshold_claim_addr = base_addr + 0x200000ULL;
    static constexpr uint64_t treshold_claim_end_addr = base_addr + 0x201007ULL;

    static constexpr uint64_t end_addr = base_addr + 0x208000ULL;

    static constexpr std::string_view peripheral_name = "PLIC";
};