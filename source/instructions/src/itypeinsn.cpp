#include "itypeinsn.hpp"

void itype::funct3(Cpu& cpu, Decoder decoder)
{
    switch (decoder.funct3())
    {
    case IType::ADDI:
        addi(cpu, decoder);
        break;
    case IType::SLLI:
        slli(cpu, decoder);
        break;
    case IType::SLTI:
        slti(cpu, decoder);
        break;
    case IType::SLTIU:
        sltiu(cpu, decoder);
        break;
    case IType::XORI:
        xori(cpu, decoder);
        break;
    case IType::SRI:
        switch (decoder.funct7() >> 1)
        {
        case IType::SRLI:
            srli(cpu, decoder);
            break;
        case IType::SRAI:
            srai(cpu, decoder);
            break;
        default:
            cpu.set_exception(exception::Exception::IllegalInstruction, decoder.insn);
            break;
        }
        break;
    case IType::ORI:
        ori(cpu, decoder);
        break;
    case IType::ANDI:
        andi(cpu, decoder);
        break;
    default:
        cpu.set_exception(exception::Exception::IllegalInstruction, decoder.insn);
        break;
    }
}

void itype::addi(Cpu& cpu, Decoder decoder)
{
    int64_t imm = decoder.imm_i();
    uint64_t rd = decoder.rd();
    uint64_t rs1 = decoder.rs1();

    cpu.regs[rd] = cpu.regs[rs1] + imm;
}

void itype::slli(Cpu& cpu, Decoder decoder)
{
    uint64_t rd = decoder.rd();
    uint64_t rs1 = decoder.rs1();
    uint32_t shamt = decoder.shamt();

    cpu.regs[rd] = cpu.regs[rs1] << shamt;
}

void itype::slti(Cpu& cpu, Decoder decoder)
{
    int64_t imm = decoder.imm_i();
    uint64_t rd = decoder.rd();
    uint64_t rs1 = decoder.rs1();

    cpu.regs[rd] = static_cast<int64_t>(cpu.regs[rs1]) < imm;
}

void itype::sltiu(Cpu& cpu, Decoder decoder)
{
    uint64_t imm = decoder.imm_i();
    uint64_t rd = decoder.rd();
    uint64_t rs1 = decoder.rs1();

    cpu.regs[rd] = cpu.regs[rs1] < imm;
}

void itype::xori(Cpu& cpu, Decoder decoder)
{
    uint64_t imm = decoder.imm_i();
    uint64_t rd = decoder.rd();
    uint64_t rs1 = decoder.rs1();

    cpu.regs[rd] = cpu.regs[rs1] ^ imm;
}

void itype::ori(Cpu& cpu, Decoder decoder)
{
    uint64_t imm = decoder.imm_i();
    uint64_t rd = decoder.rd();
    uint64_t rs1 = decoder.rs1();

    cpu.regs[rd] = cpu.regs[rs1] | imm;
}

void itype::andi(Cpu& cpu, Decoder decoder)
{
    uint64_t imm = decoder.imm_i();
    uint64_t rd = decoder.rd();
    uint64_t rs1 = decoder.rs1();

    cpu.regs[rd] = cpu.regs[rs1] & imm;
}

void itype::srli(Cpu& cpu, Decoder decoder)
{
    uint32_t imm = decoder.shamt();
    uint64_t rd = decoder.rd();
    uint64_t rs1 = decoder.rs1();

    cpu.regs[rd] = cpu.regs[rs1] >> imm;
}

void itype::srai(Cpu& cpu, Decoder decoder)
{
    uint64_t imm = decoder.imm_i();
    uint64_t rd = decoder.rd();
    uint64_t rs1 = decoder.rs1();

    cpu.regs[rd] = static_cast<int64_t>(cpu.regs[rs1]) >> imm;
}
