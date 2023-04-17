#include "bus.hpp"
#include "cpu.hpp"
#include "csrtypeinsn.hpp"
#include "decoder.hpp"
#include "ram.hpp"

#include "clint.hpp"
#include "helper.hpp"
#include "plic.hpp"
#include <exception>
#include <filesystem>
#include <fmt/core.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string.h>
#include <string_view>
#include <utility>

#if __APPLE__

#include <sys/sysctl.h>
#include <sys/types.h>
#include <unistd.h>

bool is_debugger_present()
{
    int mib[4];
    struct kinfo_proc info;
    size_t size;

    info.kp_proc.p_flag = 0;

    mib[0] = CTL_KERN;
    mib[1] = KERN_PROC;
    mib[2] = KERN_PROC_PID;
    mib[3] = getpid();

    size = sizeof(info);
    sysctl(mib, sizeof(mib) / sizeof(*mib), &info, &size, NULL, 0);

    return (info.kp_proc.p_flag & P_TRACED) != 0;
}

#elif defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)

#include <Debugapi.h>

bool is_debugger_present()
{
    return IsDebuggerPresent();
}

#elif __linux__

#include <ctype.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

bool is_debugger_present()
{
    char buf[4096];

    const int status_fd = open("/proc/self/status", O_RDONLY);
    if (status_fd == -1)
        return false;

    const ssize_t num_read = read(status_fd, buf, sizeof(buf) - 1);
    close(status_fd);

    if (num_read <= 0)
        return false;

    buf[num_read] = '\0';
    constexpr char tracerPidString[] = "TracerPid:";
    const auto tracer_pid_ptr = strstr(buf, tracerPidString);
    if (!tracer_pid_ptr)
        return false;

    for (const char* characterPtr = tracer_pid_ptr + sizeof(tracerPidString) - 1;
         characterPtr <= buf + num_read; ++characterPtr)
    {
        if (isspace(*characterPtr))
            continue;
        else
            return isdigit(*characterPtr) != 0 && *characterPtr != '0';
    }

    return false;
}

#else

bool is_debugger_present()
{
    return false;
}

#endif

#define TO_HOST_OFFSET (0x1000U)
#define TO_HOST_OFFSET_C (0x3000U)

bool test_binary(const std::filesystem::directory_entry& binary_path)
{
    RamDevice dram =
        RamDevice(0x80000000U, SIZE_KIB(64), helper::load_file(binary_path.path().c_str()));

    Cpu cpu;

    cpu.pc = dram.get_base_address();
    cpu.regs[Cpu::reg_abi_name::sp] = dram.get_end_address();

    ClintDevice clint;
    PlicDevice plic;

    cpu.bus.add_device(&dram);
    cpu.bus.add_device(&clint);
    cpu.bus.add_device(&plic);

    std::stringstream ss;

    uint64_t start = helper::get_milliseconds();
    bool timeout = false;
    bool failed_on_exception = false;
    uint64_t a0 = -1;

    bool debugger_present = is_debugger_present();

    while (!timeout)
    {
        cpu.loop(ss);

        if (dram.data[TO_HOST_OFFSET] != 0 || dram.data[TO_HOST_OFFSET_C] != 0) [[unlikely]]
        {
            a0 = cpu.regs[Cpu::reg_abi_name::a0];
            break;
        }

        if (cpu.exc_val != exception::Exception::None && cpu.cregs.regs[csr::Address::MTVEC] == 0 &&
            cpu.cregs.regs[csr::Address::STVEC] == 0) [[unlikely]]
        {
            failed_on_exception = true;
            break;
        }
        else
        {
            cpu.clear_exception();
        }

        if (!debugger_present) [[likely]]
        {
            timeout = helper::get_milliseconds() - start > 1000;
        }
    }

    if (a0 == 0)
    {
        return true;
    }

    ss << fmt::format("Test failed with pc=0x{:0>8x}\n", cpu.pc);

    if (!timeout)
    {
        if (failed_on_exception)
        {
            ss << fmt::format("Exception: {}\n\n",
                              exception::Exception::get_exception_str(cpu.exc_val));
        }
        else
        {
            ss << fmt::format("Execution finished with a0 != 0 (a0={}; gp={})\n\n", a0,
                              cpu.regs[Cpu::reg_abi_name::gp]);
        }
    }
    else
    {
        ss << "Timeout\n\n";
    }

    cpu.dump_registers(ss);

    std::string file = fmt::format("../tests/logs/{}.log", binary_path.path().stem().c_str());

    std::ofstream of = std::ofstream(file);

    if (!of)
    {
        std::cerr << fmt::format("Failed to create {}\n", file);
        return false;
    }

    of << ss.str();

    std::cout << fmt::format("Runtime data dumped to {}\n", file);

    return false;
}

bool test_bins(std::string directory)
{
    int total = 0;
    int failed = 0;
    int passed = 0;

    for (const auto& entry : std::filesystem::directory_iterator(directory))
    {
        if (std::filesystem::is_regular_file(entry) && entry.path().extension() == ".bin")
        {
            total += 1;

            std::cout << fmt::format("Performing test: {}\n", entry.path().stem().c_str());

            if (test_binary(entry))
            {
                std::cout << "Pass";
                passed += 1;
            }
            else
            {
                std::cout << "Fail";
                failed += 1;
            }

            std::cout << "\n";
        }
    }

    std::cout << fmt::format("Pass rate {:.2f}% ({}/{})\n", ((float)passed / total) * 100.0f,
                             passed, total);

    return failed == 0;
}

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        return 1;
    }

    return !test_bins(argv[1]);
}
