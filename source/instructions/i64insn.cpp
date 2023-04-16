#include "i64insn.hpp"
#include "helper.hpp"

void i64::funct3(Cpu& cpu, Decoder decoder)
{
    switch (decoder.funct3())
    {
    case I64Type::ADDIW:
        addiw(cpu, decoder);
        break;
    case I64Type::SLLIW:
        slliw(cpu, decoder);
        break;
    case I64Type::SRIW:
        switch (decoder.funct7())
        {
        case I64Type::SRLIW:
            srliw(cpu, decoder);
            break;
        case I64Type::SRAIW:
            sraiw(cpu, decoder);
            break;
        default:
            cpu.set_exception(exception::Exception::IllegalInstruction, decoder.insn);
            break;
        }
        break;
    default:
        cpu.set_exception(exception::Exception::IllegalInstruction, decoder.insn);
        break;
    }
}

void i64::addiw(Cpu& cpu, Decoder decoder)
{
    int64_t imm = decoder.imm_i();
    Cpu::reg_name rd = decoder.rd();
    Cpu::reg_name rs1 = decoder.rs1();

    cpu.regs[rd] = SIGNEXTEND_CAST(cpu.regs[rs1] + imm, int32_t);
}

void i64::slliw(Cpu& cpu, Decoder decoder)
{
    Cpu::reg_name rd = decoder.rd();
    Cpu::reg_name rs1 = decoder.rs1();
    uint64_t shamt = decoder.shamt();

    cpu.regs[rd] = SIGNEXTEND_CAST(cpu.regs[rs1] << shamt, int32_t);
}

void i64::srliw(Cpu& cpu, Decoder decoder)
{
    Cpu::reg_name rd = decoder.rd();
    Cpu::reg_name rs1 = decoder.rs1();
    uint32_t shamt = decoder.shamt();

    cpu.regs[rd] = SIGNEXTEND_CAST(static_cast<uint32_t>(cpu.regs[rs1]) >> shamt, int32_t);
}

void i64::sraiw(Cpu& cpu, Decoder decoder)
{
    Cpu::reg_name rd = decoder.rd();
    Cpu::reg_name rs1 = decoder.rs1();
    uint32_t shamt = decoder.shamt();

    cpu.regs[rd] = SIGNEXTEND_CAST(static_cast<int32_t>(cpu.regs[rs1]) >> shamt, int64_t);
}
