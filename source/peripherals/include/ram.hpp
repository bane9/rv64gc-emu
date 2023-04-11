#pragma once

#include "bus.hpp"
#include <filesystem>
#include <memory>
#include <ostream>
#include <string_view>

class RamDevice : public BusDevice
{
  public:
    RamDevice(uint64_t rom_start_address, uint64_t rom_end_offset);
    RamDevice(uint64_t rom_start_address, uint64_t rom_end_offset, std::vector<uint8_t> data);
    virtual ~RamDevice() = default;

    uint64_t load(Bus& bus, uint64_t address, uint64_t length) override;
    void store(Bus& bus, uint64_t address, uint64_t value, uint64_t length) override;

  public:
    void set_data(std::vector<uint8_t> data);

  public:
    uint64_t get_base_address() const override;
    uint64_t get_end_address() const override;

    void dump(std::ostream& stream) const override;
    std::string_view get_peripheral_name() const override;

  public:
    uint64_t base_addr;

    uint64_t end_addr;

    std::string_view peripheral_name = "RAM Segment";

  public:
    std::vector<uint8_t> data;
};
