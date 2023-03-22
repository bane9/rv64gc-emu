#pragma once

#include <cstdint>
#include <optional>
#include <ostream>
#include <span>
#include <string_view>
#include <vector>

class Bus;

class Cpu;

class BusDevice
{
  public:
    virtual ~BusDevice() = default;

    virtual uint64_t load(Bus& bus, uint64_t address, uint64_t length) = 0;
    virtual void store(Bus& bus, uint64_t address, uint64_t value, uint64_t length) = 0;

    virtual uint64_t get_base_address() const = 0;
    virtual uint64_t get_end_address() const = 0;

    virtual void tick(Cpu& cpu);
    virtual std::optional<uint32_t> is_interrupting();

    virtual std::string_view get_peripheral_name() const = 0;

    virtual void dump(std::ostream& stream) const = 0;
};

class Bus
{
  public:
    Bus() = default;
    Bus(Bus&&) = default;

    void add_device(BusDevice* bus_device);

    uint64_t load(Cpu& cpu, uint64_t address, uint64_t length);
    void store(Cpu& cpu, uint64_t address, uint64_t value, uint64_t length);

    void tick_devices(Cpu& cpu);

    std::span<BusDevice*> get_device_list();

  public:
    BusDevice* find_bus_device(uint64_t address) const;

    std::vector<BusDevice*> bus_devices;
};
