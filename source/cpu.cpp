#include "cpu.hpp"
#include "atomictype.hpp"
#include "btypeinsn.hpp"
#include "cpu_config.hpp"
#include "csrtypeinsn.hpp"
#include "ctypeinsn.hpp"
#include "decoder.hpp"
#include "fdtypeinsn.hpp"
#include "gpu.hpp"
#include "i64insn.hpp"
#include "itypeinsn.hpp"
#include "loadinsn.hpp"
#include "otherinsn.hpp"
#include "plic.hpp"
#include "queue"
#include "r64insn.hpp"
#include "ram.hpp"
#include "rtypeinsn.hpp"
#include "stypeinsn.hpp"
#include <array>
#include <fmt/core.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <utility>

Cpu::Cpu(RamDevice* dram_device, gpu::GpuDevice* gpu_device,
         virtio::VirtioBlkDevice* virtio_blk_device, SysconDevice* syscon_device)
    : mmu(*this)
{
    mode = cpu::Mode::Machine;

    pc = dram_device->get_base_address();
    regs[reg_abi_name::sp] = dram_device->get_end_address();

    bus.add_device(dram_device);

#if !NATIVE_CLI
    if (gpu_device != nullptr)
    {
        bus.add_device(gpu_device);
    }
#endif

    bus.add_device(&plic_device);
    bus.add_device(&clint_device);

#if NATIVE_CLI
    if (gpu_device != nullptr)
    {
        bus.add_device(gpu_device);
    }
#endif

    if (virtio_blk_device != nullptr)
    {
        bus.add_device(virtio_blk_device);
    }

    if (syscon_device != nullptr)
    {
        bus.add_device(syscon_device);
    }

    this->dram_device = dram_device;
    this->gpu_device = gpu_device;
    this->virtio_blk_device = virtio_blk_device;
    this->syscon_device = syscon_device;

    csr::init_handler_array();
}

void Cpu::dump_registers(std::ostream& stream)
{
    stream << "Registers:\n\n";

    for (int i = 0; i < reg_name_abi_str.size(); i++)
    {
        stream << fmt::format("{}: 0x{:0>8x}\n", reg_name_abi_str[i], regs[i]);
    }

    stream << "Float registers:\n";

    for (int i = 0; i < freg_name_abi_str.size(); i++)
    {
        stream << fmt::format("{}: {} ({:0>8x})\n", freg_name_abi_str[i], fregs[i].get_double(),
                              fregs[i].get_u64());
    }

    stream << fmt::format("pc: 0x{:0>8x}\n", pc);
    stream << fmt::format("wfi: {}\n", sleep);

    stream << "\n\n";
}

void Cpu::dump_bus_devices(std::ostream& stream)
{
    for (auto device : bus.get_device_list())
    {
        stream << fmt::format("{} (0x{:0>8x}-0x{:0>8x}):\n\n", device->get_peripheral_name(),
                              device->get_base_address(), device->get_end_address());

        device->dump(stream);

        stream << "\n\n";
    }
}

void Cpu::run()
{
    while (true)
    {
        loop(std::cout);
    }
}

void Cpu::loop(std::ostream& debug_stream)
{
    cregs.store(csr::Address::CYCLE, cregs.load(csr::Address::CYCLE) + 1);

    bus.tick_devices(*this);

    interrupt::Interrupt::InterruptValue pending_interrupt =
        interrupt::get_pending_interrupt(*this);

    if (pending_interrupt != interrupt::Interrupt::None) [[unlikely]]
    {
        if constexpr (CPU_VERBOSE_DEBUG)
        {
            debug_stream << fmt::format("Interrupt: {}\n",
                                        interrupt::Interrupt::get_interrupt_str(pending_interrupt));
        }

        interrupt::process(*this, pending_interrupt);
    }

    uint32_t insn_size = _loop(debug_stream);

    if (exc_val != exception::Exception::None) [[unlikely]]
    {
        if constexpr (CPU_VERBOSE_DEBUG)
        {
            debug_stream << fmt::format(
                "Exception: {} with data 0x{:0>8x}, happened at pc=0x{:0>8x}\n",
                exception::Exception::get_exception_str(exc_val), exc_data, pc);
        }

        exception::process(*this);

#if !CPU_TEST
        clear_exception();
#endif

        return;
    }

    // previous_pc = pc;
    pc += insn_size;
}

void Cpu::set_exception(exception::Exception::ExceptionValue value, uint64_t exc_data)
{
    exc_val = value;
    this->exc_data = exc_data;
}

void Cpu::clear_exception()
{
    exc_val = exception::Exception::None;
    exc_data = 0;
}

uint32_t Cpu::_loop(std::ostream& debug_stream)
{
    regs[reg_abi_name::zero] = 0;

    if (sleep) [[unlikely]]
    {
        return 0;
    }

    uint32_t insn = mmu.fetch(pc);

    if (exc_val != exception::Exception::None) [[unlikely]]
    {
        return 4;
    }

    Decoder decoder = Decoder(insn);

#if CPU_TEST
    debug_stream << fmt::format("pc: 0x{:0>8x}\n", pc);
    decoder.dump(debug_stream);
    debug_stream << "\n\n" << std::flush;
#endif

    if (insn == 0) [[unlikely]]
    {
        set_exception(exception::Exception::IllegalInstruction, insn);
        return 4;
    }

    uint32_t insn_size = decoder.insn_size();

    if (insn_size == 2)
    {
        execute16(decoder);
    }
    else
    {
        execute32(decoder);
    }

    return insn_size;
}

void Cpu::execute16(Decoder decoder)
{
    switch (static_cast<OpcodeType>(decoder.compressed_opcode()))
    {
    case OpcodeType::COMPRESSED_QUANDRANT0:
        ctype::quadrant0(*this, decoder);
        break;
    case OpcodeType::COMPRESSED_QUANDRANT1:
        ctype::quadrant1(*this, decoder);
        break;
    case OpcodeType::COMPRESSED_QUANDRANT2:
        ctype::quadrant2(*this, decoder);
        break;
    default:
        set_exception(exception::Exception::IllegalInstruction, decoder.insn);
        break;
    }
}

void Cpu::execute32(Decoder decoder)
{
    switch (decoder.opcode_type())
    {
    case OpcodeType::LOAD:
        load::funct3(*this, decoder);
        break;
    case OpcodeType::FENCE:
        fence::fence(*this, decoder);
        break;
    case OpcodeType::I:
        itype::funct3(*this, decoder);
        break;
    case OpcodeType::S:
        stype::funct3(*this, decoder);
        break;
    case OpcodeType::R:
        rtype::funct3(*this, decoder);
        break;
    case OpcodeType::B:
        btype::funct3(*this, decoder);
        break;
    case OpcodeType::FL:
        fdtype::fl(*this, decoder);
        break;
    case OpcodeType::FS:
        fdtype::fs(*this, decoder);
        break;
    case OpcodeType::FMADD:
        fdtype::fmadd(*this, decoder);
        break;
    case OpcodeType::FMSUB:
        fdtype::fmsub(*this, decoder);
        break;
    case OpcodeType::FNMADD:
        fdtype::fnmadd(*this, decoder);
        break;
    case OpcodeType::FNMSUB:
        fdtype::fnmsub(*this, decoder);
        break;
    case OpcodeType::FOTHER:
        fdtype::fother(*this, decoder);
        break;
    case OpcodeType::ATOMIC:
        atomic::funct3(*this, decoder);
        break;
    case OpcodeType::I64:
        i64::funct3(*this, decoder);
        break;
    case OpcodeType::R64:
        r64::funct3(*this, decoder);
        break;
    case OpcodeType::AUIPC:
        auipc::auipc(*this, decoder);
        break;
    case OpcodeType::LUI:
        lui::lui(*this, decoder);
        break;
    case OpcodeType::JAL:
        jal::jal(*this, decoder);
        break;
    case OpcodeType::JALR:
        jalr::jalr(*this, decoder);
        break;
    case OpcodeType::CSR:
        csr::funct3(*this, decoder);
        break;
    default:
        set_exception(exception::Exception::IllegalInstruction, decoder.insn);
        break;
    }
}
