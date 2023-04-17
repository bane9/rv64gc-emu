#include "csrtypeinsn.hpp"
#include "helper.hpp"
#include <array>
#include <fmt/core.h>

void csr::funct3(Cpu& cpu, Decoder decoder)
{
    switch (decoder.funct3())
    {
    case CsrType::ENVIRONMENT:
        environment(cpu, decoder);
        break;
    case CsrType::CSRW:
        csrw(cpu, decoder);
        break;
    case CsrType::CSRS:
        csrs(cpu, decoder);
        break;
    case CsrType::CSRC:
        csrc(cpu, decoder);
        break;
    case CsrType::CSRWI:
        csrwi(cpu, decoder);
        break;
    case CsrType::CSRSI:
        csrsi(cpu, decoder);
        break;
    case CsrType::CSRCI:
        csrci(cpu, decoder);
        break;
    default:
        cpu.set_exception(exception::Exception::IllegalInstruction, decoder.insn);
    }
}

void csr::environment(Cpu& cpu, Decoder decoder)
{
    uint64_t imm = decoder.rs2();
    uint64_t funct7 = decoder.funct7();

    switch (funct7)
    {
    case CsrType::SFENCEVMA7:
        sfencevma(cpu, decoder);
        return;
    case CsrType::HFENCEBVMA7:
        hfencevma(cpu, decoder);
        return;
    case CsrType::HFENCEGVMA7:
        hfencegvma(cpu, decoder);
        return;
    }

    switch (imm)
    {
    case CsrType::ECALL:
        ecall(cpu, decoder);
        break;
    case CsrType::EBREAK:
        ebreak(cpu, decoder);
        break;
    case CsrType::RET:
        switch (funct7)
        {
        case CsrType::URET7:
            uret(cpu, decoder);
            break;
        case CsrType::SRET7:
            sret(cpu, decoder);
            break;
        case CsrType::MRET7:
            mret(cpu, decoder);
            break;
        default:
            cpu.set_exception(exception::Exception::IllegalInstruction, decoder.insn);
        }
        break;
    case CsrType::WFI:
        switch (funct7)
        {
        case CsrType::WFI7:
            wfi(cpu, decoder);
            break;

        default:
            cpu.set_exception(exception::Exception::IllegalInstruction, decoder.insn);
            break;
        }
        break;
    default:
        cpu.set_exception(exception::Exception::IllegalInstruction, decoder.insn);
    }
}

void csr::ecall(Cpu& cpu, Decoder decoder)
{
    switch (cpu.mode)
    {
    case cpu::Mode::Machine:
        cpu.set_exception(exception::Exception::ECallMMode);
        break;
    case cpu::Mode::Supervisor:
        cpu.set_exception(exception::Exception::ECallSMode);
        break;
    case cpu::Mode::User:
        cpu.set_exception(exception::Exception::ECallUMode);
        break;
    default:
        cpu.set_exception(exception::Exception::IllegalInstruction, decoder.insn);
        break;
    }
}

void csr::ebreak(Cpu& cpu, Decoder decoder)
{
    cpu.set_exception(exception::Exception::Breakpoint);
}

void csr::uret(Cpu& cpu, Decoder decoder)
{
    cpu.set_exception(exception::Exception::IllegalInstruction, decoder.insn);
}

void csr::sret(Cpu& cpu, Decoder decoder)
{
    if (cpu.cregs.read_bit_mstatus(csr::Mask::MSTATUSBit::TSR) == 1 || cpu.mode == cpu::Mode::User)
    {
        cpu.set_exception(exception::Exception::IllegalInstruction, decoder.insn);
        return;
    }

    cpu.pc = cpu.cregs.load(csr::Address::SEPC) - 4;

    cpu.mode = static_cast<cpu::Mode>(cpu.cregs.read_bit_sstatus(csr::Mask::SSTATUSBit::SPP));

    if (cpu.mode == cpu::Mode::User)
    {
        cpu.cregs.write_bit_mstatus(csr::Mask::MSTATUSBit::MPRV, 0);
    }

    cpu.cregs.write_bit_sstatus(csr::Mask::SSTATUSBit::SIE,
                                cpu.cregs.read_bit_sstatus(csr::Mask::SSTATUSBit::SPIE));

    cpu.cregs.write_bit_sstatus(csr::Mask::SSTATUSBit::SPIE, 1);
    cpu.cregs.write_bit_sstatus(csr::Mask::SSTATUSBit::SPP, cpu::Mode::User);
}

void csr::mret(Cpu& cpu, Decoder decoder)
{
    if (cpu.mode != cpu::Mode::Machine)
    {
        cpu.set_exception(exception::Exception::IllegalInstruction, decoder.insn);
        return;
    }

    cpu.pc = cpu.cregs.load(csr::Address::MEPC) - 4;

    cpu.mode = cpu.cregs.read_mpp_mode();

    if (cpu.mode != cpu::Mode::Machine)
    {
        cpu.cregs.write_bit_mstatus(csr::Mask::MSTATUSBit::MPRV, 0);
    }

    cpu.cregs.write_bit_mstatus(csr::Mask::MSTATUSBit::MIE,
                                cpu.cregs.read_bit_mstatus(csr::Mask::MSTATUSBit::MPIE));

    cpu.cregs.write_bit_mstatus(csr::Mask::MSTATUSBit::MPIE, 1);
    cpu.cregs.write_mpp_mode(cpu::Mode::User);
}

void csr::wfi(Cpu& cpu, Decoder decoder)
{
#if !CPU_TEST
    // cpu.sleep = true;
#endif
}

void csr::sfencevma(Cpu& cpu, Decoder decoder)
{
    if (cpu.cregs.read_bit_mstatus(Mask::MSTATUSBit::TVM) == 1 || cpu.mode == cpu::Mode::User)
    {
        cpu.set_exception(exception::Exception::IllegalInstruction, decoder.insn);
    }

    cpu.mmu.update();
}

void csr::hfencevma(Cpu& cpu, Decoder decoder)
{
    cpu.set_exception(exception::Exception::IllegalInstruction, decoder.insn);
}

void csr::hfencegvma(Cpu& cpu, Decoder decoder)
{
    cpu.set_exception(exception::Exception::IllegalInstruction, decoder.insn);
}

using csr_op_t = uint64_t (*)(uint64_t, uint64_t);
using csr_handler_t = void (*)(Cpu&, Decoder, uint64_t, uint64_t, csr_op_t);
static std::array<csr_handler_t, 4096> csr_handlers;

static void csr_default_handler(Cpu& cpu, Decoder decoder, uint64_t csr, uint64_t rhs,
                                csr_op_t csr_op)
{
    Cpu::reg_name rd = decoder.rd();
    uint64_t csr_val = cpu.cregs.load(csr);

    cpu.cregs.store(csr, csr_op(csr_val, rhs));
    cpu.regs[rd] = csr_val;
}

static void csr_default_handler_readonly(Cpu& cpu, Decoder decoder, uint64_t csr, uint64_t rhs,
                                         csr_op_t csr_op)
{
    Cpu::reg_name rd = decoder.rd();
    uint64_t csr_val = cpu.cregs.load(csr);

    cpu.regs[rd] = csr_val;
}

static void csr_privledged_handler(Cpu& cpu, Decoder decoder, uint64_t csr, uint64_t rhs,
                                   csr_op_t csr_op)
{
    if (cpu.mode == cpu::Mode::User)
    {
        cpu.set_exception(exception::Exception::IllegalInstruction, decoder.insn);
        return;
    }

    Cpu::reg_name rd = decoder.rd();
    uint64_t csr_val = cpu.cregs.load(csr);

    cpu.cregs.store(csr, csr_op(csr_val, rhs));
    cpu.regs[rd] = csr_val;
}

static void csr_enforced_readonly_handler(Cpu& cpu, Decoder decoder, uint64_t csr, uint64_t rhs,
                                          csr_op_t csr_op)
{
    Cpu::reg_name rd = decoder.rd();
    uint64_t csr_val = cpu.cregs.load(csr);
    uint64_t opval = csr_op(csr_val, rhs);

    if (csr_val != opval)
    {
        cpu.set_exception(exception::Exception::IllegalInstruction, decoder.insn);
        return;
    }

    cpu.regs[rd] = csr_val;
}

static void csr_fcsr_handler(Cpu& cpu, Decoder decoder, uint64_t csr, uint64_t rhs, csr_op_t csr_op)
{
    Cpu::reg_name rd = decoder.rd();
    uint64_t csr_val = cpu.cregs.load(csr);
    uint64_t fexceptions = cpu.cregs.load(csr::Address::FFLAGS) & csr::FExcept::Mask;
    uint64_t op_val = csr_op(csr_val | fexceptions, rhs);

    cpu.cregs.clear_fpu_exceptions();
    cpu.cregs.set_fpu_exception(static_cast<csr::FExcept::FExceptVal>(op_val & csr::FExcept::Mask));

    cpu.cregs.store(csr::Address::FCSR, op_val & 0xff);
    cpu.regs[rd] = csr_val & 0xff;
}

static void csr_fflags_handler(Cpu& cpu, Decoder decoder, uint64_t csr, uint64_t rhs,
                               csr_op_t csr_op)
{
    Cpu::reg_name rd = decoder.rd();
    uint64_t csr_val = cpu.cregs.load(csr);
    uint64_t fexceptions = csr_val & csr::FExcept::Mask;
    uint64_t op_val = csr_op(fexceptions, rhs);

    cpu.cregs.clear_fpu_exceptions();
    cpu.cregs.set_fpu_exception(static_cast<csr::FExcept::FExceptVal>(op_val & csr::FExcept::Mask));

    uint64_t fcsr = cpu.cregs.load(csr::Address::FCSR);
    fcsr &= ~csr::FExcept::Mask;
    fcsr |= op_val & 0xff;
    cpu.cregs.store(csr::Address::FCSR, fcsr);

    cpu.regs[rd] = csr_val & csr::FExcept::Mask;
}

static void csr_frm_handler(Cpu& cpu, Decoder decoder, uint64_t csr, uint64_t rhs, csr_op_t csr_op)
{
    Cpu::reg_name rd = decoder.rd();
    uint64_t csr_val = cpu.cregs.load(csr::Address::FCSR);
    csr_val >>= 5;
    uint64_t op_val = csr_op(csr_val, rhs);

    cpu.cregs.clear_fpu_rm();
    cpu.cregs.set_fpu_rm(static_cast<FPURoundigMode::Mode>(op_val & FPURoundigMode::Mask));

    uint64_t fcsr = cpu.cregs.load(csr::Address::FCSR);
    fcsr &= ~0xe0;
    fcsr |= op_val << 5;
    fcsr &= 0xff;

    cpu.cregs.store(csr::Address::FCSR, fcsr);

    cpu.regs[rd] = csr_val & FPURoundigMode::Mask;
}

static void csr_satp_handler(Cpu& cpu, Decoder decoder, uint64_t csr, uint64_t rhs, csr_op_t csr_op)
{
    if (cpu.cregs.read_bit_mstatus(csr::Mask::MSTATUSBit::TVM) == 1)
    {
        cpu.set_exception(exception::Exception::IllegalInstruction, decoder.insn);
    }

    csr_default_handler(cpu, decoder, csr, rhs, csr_op);

    cpu.mmu.update();
}

void csr::init_handler_array()
{
    std::fill(csr_handlers.begin(), csr_handlers.end(), csr_default_handler);
    csr_handlers[Address::FCSR] = csr_fcsr_handler;
    csr_handlers[Address::FFLAGS] = csr_fflags_handler;
    csr_handlers[Address::FRM] = csr_frm_handler;
    csr_handlers[Address::SATP] = csr_satp_handler;

    csr_handlers[Address::MVENDORID] = csr_default_handler_readonly;
    csr_handlers[Address::MARCHID] = csr_default_handler_readonly;
    csr_handlers[Address::MIMPID] = csr_default_handler_readonly;
    csr_handlers[Address::MHARTID] = csr_default_handler_readonly;

    csr_handlers[Address::MISA] = csr_default_handler_readonly;
    csr_handlers[Address::TDATA1] = csr_default_handler_readonly; // Maybe supported one day

    csr_handlers[Address::CYCLE] = csr_enforced_readonly_handler;
    csr_handlers[Address::MSTATUS] = csr_privledged_handler;
}

void csr::csrw(Cpu& cpu, Decoder decoder)
{
    Cpu::reg_name rs1 = decoder.rs1();
    uint64_t rhs = cpu.regs[rs1];
    uint64_t csr = decoder.csr();

    csr_handlers[csr](
        cpu, decoder, csr, rhs, +[](uint64_t csr, uint64_t rhs) { return rhs; });
}

void csr::csrs(Cpu& cpu, Decoder decoder)
{
    Cpu::reg_name rs1 = decoder.rs1();
    uint64_t rhs = cpu.regs[rs1];
    uint64_t csr = decoder.csr();

    csr_handlers[csr](
        cpu, decoder, csr, rhs, +[](uint64_t csr, uint64_t rhs) { return csr | rhs; });
}

void csr::csrc(Cpu& cpu, Decoder decoder)
{
    Cpu::reg_name rs1 = decoder.rs1();
    uint64_t rhs = cpu.regs[rs1];
    uint64_t csr = decoder.csr();

    csr_handlers[csr](
        cpu, decoder, csr, rhs, +[](uint64_t csr, uint64_t rhs) { return csr & ~rhs; });
}

void csr::csrwi(Cpu& cpu, Decoder decoder)
{
    Cpu::reg_name rhs = decoder.rs1();
    uint64_t csr = decoder.csr();

    csr_handlers[csr](
        cpu, decoder, csr, rhs, +[](uint64_t csr, uint64_t rhs) { return rhs; });
}

void csr::csrsi(Cpu& cpu, Decoder decoder)
{
    Cpu::reg_name rhs = decoder.rs1();
    uint64_t csr = decoder.csr();

    csr_handlers[csr](
        cpu, decoder, csr, rhs, +[](uint64_t csr, uint64_t rhs) { return csr | rhs; });
}

void csr::csrci(Cpu& cpu, Decoder decoder)
{
    Cpu::reg_name rhs = decoder.rs1();
    uint64_t csr = decoder.csr();

    csr_handlers[csr](
        cpu, decoder, csr, rhs, +[](uint64_t csr, uint64_t rhs) { return csr & ~rhs; });
}
