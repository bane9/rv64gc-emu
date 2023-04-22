#include "virtio_blk.hpp"
#include "cpu.hpp"
#include "cpu_config.hpp"
#include "helper.hpp"
#include "ram.hpp"

namespace virtio
{
VirtioBlkDevice::VirtioBlkDevice(std::vector<uint8_t> data) : rfsimg(std::move(data))
{
    queue_notify = cfg::queue_notify_reset;

    vq[0].align = cfg::virtqueue_align;

    config[1] = 0x20;
    config[2] = 0x03;

    reset();
}

VirtioBlkDevice::~VirtioBlkDevice()
{
}

void VirtioBlkDevice::reset()
{
    id = 0;

    vq[0].desc = 0;
    vq[0].avail = 0;
    vq[0].used = 0;
    guest_features = {};
    queue_sel = 0;
    isr = 0;
    status = 0;
}

void VirtioBlkDevice::update()
{
    vq[0].desc = queue_pfn << guest_page_shift;
    vq[0].avail = vq[0].desc + vq[0].num * sizeof(VirtqDesc);
    vq[0].used = helper::align_up(vq[0].avail + offsetof(VRingAvail, ring[vq[0].num]), vq[0].align);
}

VirtqDesc VirtioBlkDevice::load_desc(Cpu& cpu, uint64_t address)
{
    uint64_t desc_addr = cpu.bus.load(cpu, address, 64);
    uint64_t tmp = cpu.bus.load(cpu, address + 8, 64);

    VirtqDesc desc;
    desc.addr = desc_addr;
    desc.len = tmp & 0xffffffff;
    desc.flags = (tmp >> 32) & 0xffff;
    desc.next = (tmp >> 48) & 0xffff;

    return desc;
}

void VirtioBlkDevice::access_disk(Cpu& cpu)
{
    uint64_t desc = vq[0].desc;
    uint64_t avail = vq[0].avail;
    uint64_t used = vq[0].used;

    uint64_t queue_size = vq[0].num;

    int idx = cpu.bus.load(cpu, avail + offsetof(VRingAvail, idx), 16);
    int desc_offset = cpu.bus.load(cpu, avail + 4 + (idx % queue_size) * sizeof(uint16_t), 16);

    VirtqDesc desc0 = load_desc(cpu, desc + sizeof(VirtqDesc) * desc_offset);
    VirtqDesc desc1 = load_desc(cpu, desc + sizeof(VirtqDesc) * desc0.next);
    VirtqDesc desc2 = load_desc(cpu, desc + sizeof(VirtqDesc) * desc1.next);

    int blk_req_type = cpu.bus.load(cpu, desc0.addr, 32);
    int blk_req_sector = cpu.bus.load(cpu, desc0.addr + 8, 64);

    static RamDevice* ram_device = static_cast<RamDevice*>(cpu.bus.find_bus_device(DRAM_BASE));

    if (blk_req_type == cfg::blk_t_out)
    {
        memcpy(rfsimg.data() + (blk_req_sector * cfg::sector_size),
               ram_device->data.data() + (desc1.addr - DRAM_BASE), desc1.len);
    }
    else
    {
        memcpy(ram_device->data.data() + (desc1.addr - DRAM_BASE),
               rfsimg.data() + (blk_req_sector * cfg::sector_size), desc1.len);
    }

    cpu.bus.store(cpu, desc2.addr, 8, cfg::blk_s_ok);
    cpu.bus.store(cpu, used + 4 + ((id % queue_size) * 8), 16, desc_offset);

    id++;
    cpu.bus.store(cpu, used + 2, 16, id);
}

uint64_t VirtioBlkDevice::load(Bus& bus, uint64_t address, uint64_t length)
{
    address -= base_addr;

    switch (address)
    {
    case cfg::magic_value:
        return cfg::magic;
    case cfg::version:
        return cfg::version_legacy;
    case cfg::device_id:
        return cfg::blk_dev;
    case cfg::vendor_id:
        return cfg::vendor;
    case cfg::device_features:
        return host_features[host_features_sel];
    case cfg::queue_num_max:
        return cfg::virtqueue_max_size;
    case cfg::queue_pfn:
        return queue_pfn;
    case cfg::interrupt_status:
        return isr;
    case cfg::status:
        return status;
    default:
        break;
    }

    return 0;
}

void VirtioBlkDevice::store(Bus& bus, uint64_t address, uint64_t value, uint64_t length)
{
    address -= base_addr;
    value &= 0xffffffffU;

    switch (address)
    {
    case cfg::device_features_sel: {
        host_features_sel = static_cast<bool>(value);
        break;
    }
    case cfg::driver_features: {
        guest_features[guest_features_sel] = value;
        break;
    }
    case cfg::driver_features_sel: {
        guest_features_sel = static_cast<bool>(value);
        break;
    }
    case cfg::guest_page_size: {
        if (value != 0)
        {
            guest_page_shift = __builtin_ctz(value);
        }
        break;
    }
    case cfg::queue_sel: {
        if (value != 0)
        {
            return;
        }
        break;
    }
    case cfg::queue_num: {
        if (queue_sel == 0)
        {
            vq[0].num = value;
        }
        break;
    }
    case cfg::queue_align: {
        if (queue_sel == 0)
        {
            vq[0].align = value;
        }
        break;
    }
    case cfg::queue_pfn: {
        if (queue_sel == 0)
        {
            queue_pfn = value;
            update();
        }
        break;
    }
    case cfg::queue_notify: {
        if (value == 0)
        {
            queue_notify = value;
            notify_clock = clock;
        }
        break;
    }
    case cfg::interrupt_ack: {
        isr &= ~(value & 0xff);
        break;
    }
    case cfg::status: {
        status = value & 0xff;

        if (status == 0)
        {
            reset();
        }

        if (status & 0x4)
        {
            update();
        }

        break;
    }
    default:
        break;
    }
}

void VirtioBlkDevice::tick(Cpu& cpu)
{
    if (queue_notify != cfg::queue_notify_reset && clock == notify_clock + cfg::disk_delay)
    {
        isr |= 0x1;

        access_disk(cpu);
        queue_notify = cfg::queue_notify_reset;
    }

    ++clock;
}

std::optional<uint32_t> VirtioBlkDevice::is_interrupting()
{
    if (isr & 0x1)
    {
        return cfg::virtio_irqn;
    }

    return {};
}

uint64_t VirtioBlkDevice::get_base_address() const
{
    return base_addr;
}

uint64_t VirtioBlkDevice::get_end_address() const
{
    return end_addr;
}

void VirtioBlkDevice::dump(std::ostream& stream) const
{
}

std::string_view VirtioBlkDevice::get_peripheral_name() const
{
    return peripheral_name;
}
}; // namespace virtio