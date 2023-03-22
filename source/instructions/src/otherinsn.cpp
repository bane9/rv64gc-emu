#include "otherinsn.hpp"
#include "helper.hpp"

void fence::fence(Cpu& cpu, Decoder decoder)
{
}

void jal::jal(Cpu& cpu, Decoder decoder)
{
    int64_t imm = decoder.imm_j();
    Cpu::reg_name rd = decoder.rd();

    cpu.regs[rd] = cpu.pc + 4;
    cpu.pc += imm - 4;
}

void jalr::jalr(Cpu& cpu, Decoder decoder)
{
    int64_t imm = decoder.imm_i();
    Cpu::reg_name rs1 = decoder.rs1();
    Cpu::reg_name rd = decoder.rd();

    uint64_t tmp = cpu.pc + 4;
    int64_t val = cpu.regs[rs1];

    cpu.pc = ((val + imm) & ~1) - 4;

    cpu.regs[rd] = tmp;
}

void auipc::auipc(Cpu& cpu, Decoder decoder)
{
    Cpu::reg_name rd = decoder.rd();
    int64_t imm = SIGNEXTEND_CAST(decoder.insn & 0xfffff000, int32_t);

    cpu.regs[rd] = cpu.pc + imm;
}

void lui::lui(Cpu& cpu, Decoder decoder)
{
    Cpu::reg_name rd = decoder.rd();

    cpu.regs[rd] = SIGNEXTEND_CAST(decoder.insn & 0xfffff000, int32_t);
}
