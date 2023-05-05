#include "ram.hpp"
#include "helper.hpp"
#include <cstdlib>
#include <fstream>

RamDevice::RamDevice(uint64_t rom_start_address, uint64_t rom_end_offset)
{
    base_addr = rom_start_address;
    end_addr = rom_start_address + rom_end_offset;

    data.resize(end_addr - base_addr);
}

RamDevice::RamDevice(uint64_t rom_start_address, uint64_t rom_end_offset, std::vector<uint8_t> data)
{
    base_addr = rom_start_address;
    end_addr = rom_start_address + rom_end_offset;

    this->data.resize(end_addr - base_addr);
    set_data(std::move(data));
}

uint64_t RamDevice::load(Bus& bus, uint64_t address, uint64_t length)
{
    address -= base_addr;

    switch (length)
    {
    case 8:
        return data[address];
    case 16:
        return *reinterpret_cast<uint16_t*>(data.data() + address);
    case 32:
        return *reinterpret_cast<uint32_t*>(data.data() + address);
    case 64:
        return *reinterpret_cast<uint64_t*>(data.data() + address);
    default:
        break;
    }

    return 0;
}

void RamDevice::set_data(std::vector<uint8_t> data)
{
    memcpy(this->data.data(), data.data(), data.size());
}

void RamDevice::store(Bus& bus, uint64_t address, uint64_t value, uint64_t length)
{
    address -= base_addr;

    switch (length)
    {
    case 8:
        data[address] = value;
        break;
    case 16:
        *reinterpret_cast<uint16_t*>(data.data() + address) = value;
        break;
    case 32:
        *reinterpret_cast<uint32_t*>(data.data() + address) = value;
        break;
    case 64:
        *reinterpret_cast<uint64_t*>(data.data() + address) = value;
        break;
    default:
        break;
    }
}

void RamDevice::dump(std::ostream& stream) const
{
    helper::dump_hex(stream, data.data(), data.size());
}

uint64_t RamDevice::get_base_address() const
{
    return base_addr;
}

uint64_t RamDevice::get_end_address() const
{
    return end_addr;
}

std::string_view RamDevice::get_peripheral_name() const
{
    return peripheral_name;
}
