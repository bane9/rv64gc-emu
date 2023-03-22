#include "loadinsn.hpp"
#include "helper.hpp"

void load::funct3(Cpu& cpu, Decoder decoder)
{
    switch (decoder.funct3())
    {
    case LOADType::LB:
        load::lb(cpu, decoder);
        break;
    case LOADType::LH:
        load::lh(cpu, decoder);
        break;
    case LOADType::LW:
        load::lw(cpu, decoder);
        break;
    case LOADType::LD:
        load::ld(cpu, decoder);
        break;
    case LOADType::LBU:
        load::lbu(cpu, decoder);
        break;
    case LOADType::LHU:
        load::lhu(cpu, decoder);
        break;
    case LOADType::LWU:
        load::lwu(cpu, decoder);
        break;
    default:
        cpu.set_exception(exception::Exception::IllegalInstruction, decoder.insn);
        break;
    }
}

#define ASSIGN_IF_NO_EXCEPTION(rd, load_expr)                                                      \
    uint64_t _check_temp = (load_expr);                                                            \
    if (cpu.exc_val == exception::Exception::None) [[likely]]                                      \
    {                                                                                              \
        cpu.regs[(rd)] = _check_temp;                                                              \
    }

void load::lb(Cpu& cpu, Decoder insn)
{
    int64_t imm = insn.imm_i();
    Cpu::reg_name rs1 = insn.rs1();
    Cpu::reg_name rd = insn.rd();
    uint64_t addr = cpu.regs[rs1] + imm;

    ASSIGN_IF_NO_EXCEPTION(rd, SIGNEXTEND_CAST2(cpu.mmu.load(addr, 8), int8_t));
}

void load::lh(Cpu& cpu, Decoder insn)
{
    int64_t imm = insn.imm_i();
    Cpu::reg_name rs1 = insn.rs1();
    Cpu::reg_name rd = insn.rd();
    uint64_t addr = cpu.regs[rs1] + imm;

    ASSIGN_IF_NO_EXCEPTION(rd, SIGNEXTEND_CAST2(cpu.mmu.load(addr, 16), int16_t));
}

void load::lw(Cpu& cpu, Decoder insn)
{
    int64_t imm = insn.imm_i();
    Cpu::reg_name rs1 = insn.rs1();
    Cpu::reg_name rd = insn.rd();
    uint64_t addr = cpu.regs[rs1] + imm;

    ASSIGN_IF_NO_EXCEPTION(rd, SIGNEXTEND_CAST2(cpu.mmu.load(addr, 32), int32_t));
}

void load::ld(Cpu& cpu, Decoder insn)
{
    int64_t imm = insn.imm_i();
    Cpu::reg_name rs1 = insn.rs1();
    Cpu::reg_name rd = insn.rd();
    uint64_t addr = cpu.regs[rs1] + imm;

    ASSIGN_IF_NO_EXCEPTION(rd, SIGNEXTEND_CAST2(cpu.mmu.load(addr, 64), int64_t));
}

void load::lbu(Cpu& cpu, Decoder insn)
{
    int64_t imm = insn.imm_i();
    Cpu::reg_name rs1 = insn.rs1();
    Cpu::reg_name rd = insn.rd();
    uint64_t addr = cpu.regs[rs1] + imm;

    ASSIGN_IF_NO_EXCEPTION(rd, cpu.mmu.load(addr, 8));
}

void load::lhu(Cpu& cpu, Decoder insn)
{
    int64_t imm = insn.imm_i();
    Cpu::reg_name rs1 = insn.rs1();
    Cpu::reg_name rd = insn.rd();
    uint64_t addr = cpu.regs[rs1] + imm;

    ASSIGN_IF_NO_EXCEPTION(rd, cpu.mmu.load(addr, 16));
}

void load::lwu(Cpu& cpu, Decoder insn)
{
    int64_t imm = insn.imm_i();
    Cpu::reg_name rs1 = insn.rs1();
    Cpu::reg_name rd = insn.rd();
    uint64_t addr = cpu.regs[rs1] + imm;

    ASSIGN_IF_NO_EXCEPTION(rd, cpu.mmu.load(addr, 32));
}
