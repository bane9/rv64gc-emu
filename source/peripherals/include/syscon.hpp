#pragma once

#include "bus.hpp"
#include <ostream>
#include <string_view>

class SysconDevice : public BusDevice
{
  public:
    SysconDevice() = default;
    virtual ~SysconDevice() = default;

    uint64_t load(Bus& bus, uint64_t address, uint64_t length) override;
    void store(Bus& bus, uint64_t address, uint64_t value, uint64_t length) override;

  public:
    uint64_t get_base_address() const override;
    uint64_t get_end_address() const override;

    void dump(std::ostream& stream) const override;
    std::string_view get_peripheral_name() const override;

  public:
    static constexpr uint64_t poweroff_addr = 0x5555;

    static constexpr uint64_t reboot_addr = 0x7777;

    static constexpr uint64_t base_addr = poweroff_addr;

    static constexpr uint64_t end_addr = reboot_addr;

    static constexpr std::string_view peripheral_name = "SYSCON";
};
