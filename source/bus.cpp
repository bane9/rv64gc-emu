#include "bus.hpp"

#include "clint.hpp"
#include "cpu.hpp"
#include "gpu.hpp"
#include <algorithm>
#include <fmt/core.h>

void Bus::add_device(BusDevice* bus_device)
{
    bus_devices.push_back(bus_device);
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

    for (size_t i = 0; i < bus_devices.size(); i++)
    {
        BusDevice* device = devices[i];

        if (address >= device->get_base_address() && address < device->get_end_address())
        {
            return device;
        }
    }

    return nullptr;
}

void Bus::tick_devices(Cpu& cpu)
{
#if !CPU_TEST
    static gpu::GpuDevice* gpu_device =
        static_cast<gpu::GpuDevice*>(find_bus_device(gpu::cfg::uart_base_address));

    static ClintDevice* clint_device = static_cast<ClintDevice*>(find_bus_device(clint_base_addr));

    gpu_device->tick(cpu);
    clint_device->tick(cpu);
#endif
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