#include "bus.hpp"
#include "clint.hpp"
#include "cpu.hpp"
#include "gpu.hpp"
#include "helper.hpp"
#include "plic.hpp"
#include "ram.hpp"
#include <algorithm>
#include <array>
#include <cstring>
#include <filesystem>
#include <fmt/core.h>
#include <getopt.h>
#include <iostream>
#include <utility>

void print_usage(char* argv[])
{
    std::cerr << fmt::format(
        "Usage: {} [options]\n"
        "Options:\n"
        "  -b, --bios   Path to the BIOS file (mandatory)\n"
#if !NATIVE_CLI
        "  -f, --font   Path to the font file (mandatory)\n"
#endif
        "  -d, --dtb    Path to the device tree blob file (optional, "
        "mandatory if kernel is present)\n"
        "  -k, --kernel Path to the kernel file (optional)\n"
        "  -m, --memory Emulator RAM buffer size in MiB (optional, default 64 MiB)\n",
        argv[0]);
}

void error_exit(char* argv[], const std::string& error_message)
{
    std::cerr << fmt::format("Error: {}\n", error_message);
    print_usage(argv);
    exit(1);
}

bool file_exists(const char* path)
{
    return std::filesystem::exists(path);
}

bool patch_dtb_ram_size(std::vector<uint8_t>& dtb_data, uint32_t ram_size)
{
    static constexpr std::array<uint8_t, sizeof(uint32_t)> search_pattern = {0x0b, 0xad, 0xc0,
                                                                             0xde};

    auto it =
        std::search(dtb_data.begin(), dtb_data.end(), search_pattern.begin(), search_pattern.end());

    if (it == dtb_data.end())
    {
        return false;
    }

    uint32_t ram_size_be = ((ram_size & 0xff000000U) >> 24U) | ((ram_size & 0x00ff0000U) >> 8U) |
                           ((ram_size & 0x0000ff00U) << 8U) | ((ram_size & 0x000000ffU) << 24U);

    memcpy(&*it, &ram_size_be, sizeof(ram_size_be));

    return true;
}

int main(int argc, char* argv[])
{
    const char* bios_path = nullptr;
    const char* font_path = nullptr;
    const char* dtb_path = nullptr;
    const char* kernel_path = nullptr;

    uint64_t ram_size = SIZE_MIB(64);

    // clang-format off
    static constexpr option long_options[] = {
        {"bios", required_argument, nullptr, 'b'},
        {"font", required_argument, nullptr, 'f'},
        {"dtb", required_argument, nullptr, 'd'},
        {"kernel", required_argument, nullptr, 'k'},
        {"memory", required_argument, nullptr, 'm'},
        {}
    };
    // clang-format on

    int opt;
    int option_index = 0;

    while ((opt = getopt_long(argc, argv, "b:f:d:k:m:", long_options, &option_index)) != -1)
    {
        switch (opt)
        {
        case 'b':
            bios_path = optarg;
            break;
        case 'f':
            font_path = optarg;
            break;
        case 'd':
            dtb_path = optarg;
            break;
        case 'k':
            kernel_path = optarg;
            break;
        case 'm':
            ram_size = SIZE_MIB(atoi(optarg));
            break;
        default:
            print_usage(argv);
            exit(1);
        }
    }

    if (bios_path == nullptr
#if !NATIVE_CLI
        || font_path == nullptr
#endif
    )
    {
#if !NATIVE_CLI
        error_exit(argv, "both bios_path and font_path must be provided");
#else
        error_exit(argv, "bios_path must be provided");
#endif
    }

    if (kernel_path != nullptr && dtb_path == nullptr)
    {
        error_exit(argv, "dtb path must be provided when kernel path is provided");
    }

    if (!file_exists(bios_path))
    {
        error_exit(argv, "bios path invalid");
    }

#if !NATIVE_CLI
    if (!file_exists(font_path))
    {
        error_exit(argv, "font path invalid");
    }
#endif

    uint64_t ram_size_total = ram_size;

    if (dtb_path != nullptr)
    {
        ram_size_total += SIZE_MIB(2);
    }

    auto bios = helper::load_file(bios_path);
    RamDevice dram = RamDevice(0x80000000U, ram_size_total, std::move(bios));

    Cpu cpu;

    cpu.pc = dram.get_base_address();
    cpu.regs[Cpu::reg_abi_name::sp] = dram.get_base_address() + ram_size;

    gpu::GpuDevice gpu = gpu::GpuDevice("RISC V emulator", font_path, 960, 540);

    if (dtb_path)
    {
        if (!file_exists(dtb_path))
        {
            error_exit(argv, "dtb path invalid");
        }

        std::vector<uint8_t> dtb = helper::load_file(dtb_path);
        uint64_t dtb_offset = dram.data.size() - SIZE_MIB(2);
        cpu.regs[Cpu::reg_abi_name::a1] = dram.get_base_address() + dtb_offset;

        if (!patch_dtb_ram_size(dtb, ram_size_total))
        {
            std::cout << "Warning: couldn't find dtb memory size magic value (0x0badc0de),"
                         "allocating 66MiB (64MiB system, 2MiB dtb)\n";
        }

        memcpy(dram.data.data() + dtb_offset, dtb.data(), dtb.size());
    }

    if (kernel_path)
    {
        if (!file_exists(kernel_path))
        {
            error_exit(argv, "kernel path invalid");
        }

        std::vector<uint8_t> kernel = helper::load_file(kernel_path);
        memcpy(dram.data.data() + 0x200000U, kernel.data(), kernel.size());
    }

    ClintDevice clint;
    PlicDevice plic;

    cpu.bus.add_device(&dram);
    cpu.bus.add_device(&gpu);
    cpu.bus.add_device(&clint);
    cpu.bus.add_device(&plic);

    cpu.run();
}
