#include "bus.hpp"

#include "cpu.hpp"
#include <algorithm>
#include <fmt/core.h>

void Bus::add_device(BusDevice* bus_device)
{
    bus_devices.push_back(bus_device);

    std::sort(bus_devices.begin(), bus_devices.end(), [](BusDevice* dev1, BusDevice* dev2) {
        return dev1->get_base_address() < dev2->get_base_address();
    });
}

uint64_t Bus::load(Cpu& cpu, uint64_t address, uint64_t length)
{
    BusDevice* bus_device = find_bus_device(address);

    if (bus_device != nullptr)
    {
        return bus_device->load(*this, address, length);
    }
    else
    {
        cpu.set_exception(exception::Exception::LoadAccessFault);
        return 0;
    }
}

void Bus::store(Cpu& cpu, uint64_t address, uint64_t value, uint64_t length)
{
    BusDevice* bus_device = find_bus_device(address);

    if (bus_device != nullptr)
    {
        bus_device->store(*this, address, value, length);
    }
    else
    {
        cpu.set_exception(exception::Exception::StoreAccessFault);
    }
}

BusDevice* Bus::find_bus_device(uint64_t address) const
{
    auto devices = bus_devices.data();

    for (int i = 0; i < bus_devices.size(); i++)
    {
        if (address >= devices[i]->get_base_address() && address < devices[i]->get_end_address())
        {
            return devices[i];
        }
    }

    return nullptr;
}

void Bus::tick_devices(Cpu& cpu)
{
    for (BusDevice* device : bus_devices)
    {
        device->tick(cpu);
    }
}

std::span<BusDevice*> Bus::get_device_list()
{
    return bus_devices;
}

void BusDevice::tick([[maybe_unused]] Cpu& cpu)
{
}

std::optional<uint32_t> BusDevice::is_interrupting()
{
    return {};
}