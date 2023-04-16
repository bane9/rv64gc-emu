#include "stypeinsn.hpp"

#include "helper.hpp"

void stype::funct3(Cpu& cpu, Decoder decoder)
{
    switch (decoder.funct3())
    {
    case SType::SB:
        sb(cpu, decoder);
        break;
    case SType::SH:
        sh(cpu, decoder);
        break;
    case SType::SW:
        sw(cpu, decoder);
        break;
    case SType::SD:
        sd(cpu, decoder);
        break;
    default:
        cpu.set_exception(exception::Exception::IllegalInstruction, decoder.insn);
        break;
    }
}

void stype::sb(Cpu& cpu, Decoder decoder)
{
    int64_t imm = decoder.imm_s();
    uint64_t rs1 = decoder.rs1();
    uint64_t rs2 = decoder.rs2();
    uint64_t address = cpu.regs[rs1] + imm;
    uint64_t value = cpu.regs[rs2];

    cpu.mmu.store(address, value, 8);
}

void stype::sh(Cpu& cpu, Decoder decoder)
{
    int64_t imm = decoder.imm_s();
    uint64_t rs1 = decoder.rs1();
    uint64_t rs2 = decoder.rs2();

    uint64_t address = cpu.regs[rs1] + imm;
    uint64_t value = cpu.regs[rs2];

    cpu.mmu.store(address, value, 16);
}

void stype::sw(Cpu& cpu, Decoder decoder)
{
    int64_t imm = decoder.imm_s();
    uint64_t rs1 = decoder.rs1();
    uint64_t rs2 = decoder.rs2();
    uint64_t address = cpu.regs[rs1] + imm;
    uint64_t value = cpu.regs[rs2];

    cpu.mmu.store(address, value, 32);
}

void stype::sd(Cpu& cpu, Decoder decoder)
{
    int64_t imm = decoder.imm_s();
    uint64_t rs1 = decoder.rs1();
    uint64_t rs2 = decoder.rs2();
    uint64_t address = cpu.regs[rs1] + imm;
    uint64_t value = cpu.regs[rs2];

    cpu.mmu.store(address, value, 64);
}
