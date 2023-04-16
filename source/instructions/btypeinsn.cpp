#include "btypeinsn.hpp"
#include "helper.hpp"

void btype::funct3(Cpu& cpu, Decoder decoder)
{
    switch (decoder.funct3())
    {
    case BType::BEQ:
        beq(cpu, decoder);
        break;
    case BType::BNE:
        bne(cpu, decoder);
        break;
    case BType::BLT:
        blt(cpu, decoder);
        break;
    case BType::BGE:
        bge(cpu, decoder);
        break;
    case BType::BLTU:
        bltu(cpu, decoder);
        break;
    case BType::BGEU:
        bgeu(cpu, decoder);
        break;
    default:
        cpu.set_exception(exception::Exception::IllegalInstruction, decoder.insn);
        break;
    }
}

void btype::beq(Cpu& cpu, Decoder decoder)
{
    int64_t imm = decoder.imm_b();
    Cpu::reg_name rs1 = decoder.rs1();
    Cpu::reg_name rs2 = decoder.rs2();

    if (static_cast<int64_t>(cpu.regs[rs1]) == static_cast<int64_t>(cpu.regs[rs2]))
    {
        cpu.pc += imm - 4;
    }
}

void btype::bne(Cpu& cpu, Decoder decoder)
{
    int64_t imm = decoder.imm_b();
    Cpu::reg_name rs1 = decoder.rs1();
    Cpu::reg_name rs2 = decoder.rs2();

    if (static_cast<int64_t>(cpu.regs[rs1]) != static_cast<int64_t>(cpu.regs[rs2]))
    {
        cpu.pc += imm - 4;
    }
}

void btype::blt(Cpu& cpu, Decoder decoder)
{
    int64_t imm = decoder.imm_b();
    Cpu::reg_name rs1 = decoder.rs1();
    Cpu::reg_name rs2 = decoder.rs2();

    if (static_cast<int64_t>(cpu.regs[rs1]) < static_cast<int64_t>(cpu.regs[rs2]))
    {
        cpu.pc += imm - 4;
    }
}

void btype::bge(Cpu& cpu, Decoder decoder)
{
    int64_t imm = decoder.imm_b();
    Cpu::reg_name rs1 = decoder.rs1();
    Cpu::reg_name rs2 = decoder.rs2();

    if (static_cast<int64_t>(cpu.regs[rs1]) >= static_cast<int64_t>(cpu.regs[rs2]))
    {
        cpu.pc += imm - 4;
    }
}

void btype::bltu(Cpu& cpu, Decoder decoder)
{
    int64_t imm = decoder.imm_b();
    Cpu::reg_name rs1 = decoder.rs1();
    Cpu::reg_name rs2 = decoder.rs2();

    if (cpu.regs[rs1] < cpu.regs[rs2])
    {
        cpu.pc += imm - 4;
    }
}

void btype::bgeu(Cpu& cpu, Decoder decoder)
{
    int64_t imm = decoder.imm_b();
    Cpu::reg_name rs1 = decoder.rs1();
    Cpu::reg_name rs2 = decoder.rs2();

    if (cpu.regs[rs1] >= cpu.regs[rs2])
    {
        cpu.pc += imm - 4;
    }
}
