#pragma once

#include "bus.hpp"
#include "cpu_config.hpp"
#include <array>
#include <limits>
#include <string_view>
#include <vector>

namespace virtio
{

namespace cfg
{
constexpr uint64_t virtio_base_address = 0x10001000ULL;
constexpr uint64_t virtio_size = 0x1000ULL;

constexpr uint64_t virtio_irqn = 0x01;

constexpr uint64_t magic_value = 0x0;
constexpr uint64_t version = 0x4;
constexpr uint64_t device_id = 0x8;
constexpr uint64_t vendor_id = 0xc;
constexpr uint64_t device_features = 0x10;
constexpr uint64_t device_features_sel = 0x14;
constexpr uint64_t driver_features = 0x20;
constexpr uint64_t driver_features_sel = 0x24;
constexpr uint64_t guest_page_size = 0x28;
constexpr uint64_t queue_sel = 0x30;
constexpr uint64_t queue_notify_reset = std::numeric_limits<uint32_t>::max();
constexpr uint64_t queue_num_max = 0x34;
constexpr uint64_t queue_num = 0x38;
constexpr uint64_t queue_align = 0x3c;
constexpr uint64_t queue_pfn = 0x40;
constexpr uint64_t queue_notify = 0x50;
constexpr uint64_t interrupt_status = 0x60;
constexpr uint64_t interrupt_ack = 0x64;
constexpr uint64_t status = 0x70;
constexpr uint64_t config = 0x100;

constexpr uint32_t magic = 0x74726976;
constexpr uint32_t version_legacy = 0x1;
constexpr uint32_t vendor = 0x554D4551;
constexpr uint32_t blk_dev = 0x02;

constexpr uint16_t virtqueue_max_size = 0x400;
constexpr uint16_t virtqueue_align = 0x1000;

constexpr uint32_t disk_delay = 0x1f4;
constexpr uint32_t sector_size = 0x200;

constexpr uint8_t blk_t_in = 0x0;
constexpr uint8_t blk_t_out = 0x1;

constexpr uint16_t desc_f_next = 0x1;
constexpr uint16_t desc_f_write = 0x2;

constexpr uint8_t blk_s_ok = 0x0;
}; // namespace cfg

struct VRingAvail
{
    uint16_t flags;
    uint16_t idx;
    uint16_t ring[];
};

struct VirtqDesc
{
    uint64_t addr;
    uint32_t len;
    uint16_t flags;
    uint16_t next;
};

struct Virtq
{
    uint32_t num;
    uint32_t align;

    uint64_t desc;
    uint64_t avail;
    uint64_t used;
};

class VirtioBlkDevice : public BusDevice
{
  public:
    VirtioBlkDevice(std::vector<uint8_t> data);
    virtual ~VirtioBlkDevice();

    uint64_t load(Bus& bus, uint64_t address, uint64_t length) override;
    void store(Bus& bus, uint64_t address, uint64_t value, uint64_t length) override;

  public:
    void tick(Cpu& cpu) override;
    std::optional<uint32_t> is_interrupting() override;

  public:
    uint64_t get_base_address() const override;
    uint64_t get_end_address() const override;

    void dump(std::ostream& stream) const override;

    std::string_view get_peripheral_name() const override;

  public:
    void reset();
    void update();
    VirtqDesc load_desc(Cpu& cpu, uint64_t address);
    void access_disk(Cpu& cpu);

  public:
    uint64_t id = 0;
    uint64_t clock = 0;
    uint64_t notify_clock = 0;

    std::array<Virtq, 1> vq = {};
    uint16_t queue_sel = 0;
    std::array<uint32_t, 2> host_features = {};
    std::array<uint32_t, 2> guest_features = {};
    uint32_t host_features_sel = 0;
    uint32_t guest_features_sel = 0;
    uint32_t guest_page_shift = 0;
    uint32_t queue_pfn = 0;
    uint32_t queue_notify = 0;
    uint8_t isr = 0;
    uint8_t status = 0;
    std::array<uint8_t, 8> config = {};

    std::vector<uint8_t> rfsimg;

  public:
    static constexpr uint64_t base_addr = cfg::virtio_base_address;

    static constexpr uint64_t end_addr = base_addr + cfg::virtio_size;

    static constexpr std::string_view peripheral_name = "VIRTIO BLK";
};
} // namespace virtio
