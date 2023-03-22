#pragma once

#include "bus.hpp"
#include "cpu_config.hpp"

class ClintDevice : public BusDevice
{
  public:
    ClintDevice();
    virtual ~ClintDevice() = default;

    virtual uint64_t load(Bus& bus, uint64_t address, uint64_t length) override;
    virtual void store(Bus& bus, uint64_t address, uint64_t value, uint64_t length) override;

  public:
    void tick(Cpu& cpu) override;

  public:
    virtual uint64_t get_base_address() const override;
    virtual uint64_t get_end_address() const override;

    virtual void dump(std::ostream& stream) const override;

    virtual std::string_view get_peripheral_name() const override;

  public:
    uint32_t msip = 0;
    uint64_t mtimecmp = 0;
    uint64_t mtime = 0;
    uint64_t last_ms;

  public:
    static constexpr uint64_t base_addr = clint_base_addr;

    static constexpr uint64_t msip_addr = base_addr;
    static constexpr uint64_t mtimecmp_addr = base_addr + 0x4000ULL;
    static constexpr uint64_t mtime_addr = base_addr + 0xbff8ULL;

    static constexpr uint64_t end_addr = base_addr + 0x10000ULL;

    static constexpr std::string_view peripheral_name = "CLINT";
};