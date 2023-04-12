#include "bus.hpp"
#include "clint.hpp"
#include "cpu.hpp"
#include "gpu.hpp"
#include "helper.hpp"
#include "plic.hpp"
#include "ram.hpp"
#include <cstring>
#include <filesystem>
#include <fmt/core.h>
#include <getopt.h>
#include <iostream>
#include <utility>

void print_usage(char* argv[])
{
    std::cerr << fmt::format("Usage: {} [options]\n"
                             "Options:\n"
                             "  -b, --bios bios_path     Path to the BIOS file (mandatory)\n"
#if !NATIVE_CLI
                             "  -f, --font font_path     Path to the font file (mandatory)\n"
#endif
                             "  -d, --dtb dtb_path       Path to the device tree blob file "
                             "(optional, mandatory if kernel is present)\n"
                             "  -k, --kernel kernel_path Path to the kernel file (optional)\n",
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

int main(int argc, char* argv[])
{
    const char* bios_path = nullptr;
    const char* font_path = nullptr;
    const char* dtb_path = nullptr;
    const char* kernel_path = nullptr;

    static constexpr option long_options[] = {{"bios", required_argument, nullptr, 'b'},
                                              {"font", required_argument, nullptr, 'f'},
                                              {"dtb", required_argument, nullptr, 'd'},
                                              {"kernel", required_argument, nullptr, 'k'},
                                              {nullptr, 0, nullptr, 0}};

    int opt;
    int option_index = 0;

    while ((opt = getopt_long(argc, argv, "b:f:d:k:", long_options, &option_index)) != -1)
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

    auto bios = helper::load_file(bios_path);
    RamDevice dram = RamDevice(0x80000000U, SIZE_MIB(256) + sizeof(uint64_t), std::move(bios));

    Cpu cpu = Cpu(dram.get_base_address(), dram.get_end_address());

    gpu::GpuDevice gpu = gpu::GpuDevice("RISC V emulator", font_path, 960, 540);

    RamDevice* dtb_rom = nullptr;

    if (dtb_path)
    {
        if (!file_exists(dtb_path))
        {
            error_exit(argv, "dtb path invalid");
        }

        dtb_rom = new RamDevice(0x10000, 0xe000);

        dtb_rom->set_data(helper::load_file(dtb_path));
        cpu.regs[Cpu::reg_abi_name::a1] = dtb_rom->get_base_address();
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

    if (dtb_rom != nullptr)
    {
        cpu.bus.add_device(dtb_rom);
    }

    cpu.run();
}
