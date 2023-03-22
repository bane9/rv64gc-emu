#include "r64insn.hpp"

#include "helper.hpp"
#include <limits>

void r64::funct3(Cpu& cpu, Decoder decoder)
{
    switch (decoder.funct3())
    {
    case R64Type::ADDSUBW:
        switch (decoder.funct7())
        {
        case R64Type::ADDW:
            addw(cpu, decoder);
            break;
        case R64Type::MULW:
            mulw(cpu, decoder);
            break;
        case R64Type::SUBW:
            subw(cpu, decoder);
            break;
        default:
            cpu.set_exception(exception::Exception::IllegalInstruction, decoder.insn);
            break;
        }
        break;
    case R64Type::DIVW:
        divw(cpu, decoder);
        break;
    case R64Type::SLLW:
        sllw(cpu, decoder);
        break;
    case R64Type::SRW:
        switch (decoder.funct7())
        {
        case R64Type::SRLW:
            srlw(cpu, decoder);
            break;
        case R64Type::DIVUW:
            divuw(cpu, decoder);
            break;
        case R64Type::SRAW:
            sraw(cpu, decoder);
            break;
        default:
            cpu.set_exception(exception::Exception::IllegalInstruction, decoder.insn);
            break;
        }
        break;
    case R64Type::REMW:
        remw(cpu, decoder);
        break;
    case R64Type::REMUW:
        remuw(cpu, decoder);
        break;
    default:
        cpu.set_exception(exception::Exception::IllegalInstruction, decoder.insn);
        break;
    }
}

void r64::addw(Cpu& cpu, Decoder decoder)
{
    Cpu::reg_name rd = decoder.rd();
    Cpu::reg_name rs1 = decoder.rs1();
    Cpu::reg_name rs2 = decoder.rs2();

    uint64_t val1 = cpu.regs[rs1];
    int64_t val2 = cpu.regs[rs2];

    cpu.regs[rd] = SIGNEXTEND_CAST(val1 + val2, int32_t);
}

void r64::mulw(Cpu& cpu, Decoder decoder)
{
    Cpu::reg_name rd = decoder.rd();
    Cpu::reg_name rs1 = decoder.rs1();
    Cpu::reg_name rs2 = decoder.rs2();

    uint64_t val1 = cpu.regs[rs1];
    int64_t val2 = cpu.regs[rs2];

    cpu.regs[rd] = SIGNEXTEND_CAST(val1 * val2, int32_t);
}

void r64::subw(Cpu& cpu, Decoder decoder)
{
    Cpu::reg_name rd = decoder.rd();
    Cpu::reg_name rs1 = decoder.rs1();
    Cpu::reg_name rs2 = decoder.rs2();

    uint64_t val1 = cpu.regs[rs1];
    int64_t val2 = cpu.regs[rs2];

    cpu.regs[rd] = SIGNEXTEND_CAST(val1 - val2, int32_t);
}

void r64::divw(Cpu& cpu, Decoder decoder)
{
    Cpu::reg_name rd = decoder.rd();
    Cpu::reg_name rs1 = decoder.rs1();
    Cpu::reg_name rs2 = decoder.rs2();

    int32_t val1 = cpu.regs[rs1];
    int32_t val2 = cpu.regs[rs2];

    if (val2 == 0)
    {
        cpu.regs[rd] = ~0ULL;
    }
    else if (val1 == std::numeric_limits<int32_t>::min() && val2 == -1)
    {
        cpu.regs[rd] = static_cast<int64_t>(val1);
    }
    else
    {
        cpu.regs[rd] = SIGNEXTEND_CAST(val1 / val2, int64_t);
    }
}

void r64::sllw(Cpu& cpu, Decoder decoder)
{
    Cpu::reg_name rd = decoder.rd();
    Cpu::reg_name rs1 = decoder.rs1();
    Cpu::reg_name rs2 = decoder.rs2();

    uint64_t val1 = cpu.regs[rs1];
    uint64_t val2 = cpu.regs[rs2] & 0x1f;

    cpu.regs[rd] = SIGNEXTEND_CAST(val1 << val2, int32_t);
}

void r64::srlw(Cpu& cpu, Decoder decoder)
{
    Cpu::reg_name rd = decoder.rd();
    Cpu::reg_name rs1 = decoder.rs1();
    Cpu::reg_name rs2 = decoder.rs2();

    uint32_t val1 = cpu.regs[rs1];
    uint64_t val2 = cpu.regs[rs2] & 0x1f;

    cpu.regs[rd] = SIGNEXTEND_CAST(val1 >> val2, int32_t);
}

void r64::divuw(Cpu& cpu, Decoder decoder)
{
    Cpu::reg_name rd = decoder.rd();
    Cpu::reg_name rs1 = decoder.rs1();
    Cpu::reg_name rs2 = decoder.rs2();

    uint32_t val1 = cpu.regs[rs1];
    uint32_t val2 = cpu.regs[rs2];

    if (val2 == 0)
    {
        cpu.regs[rd] = ~0ULL;
    }
    else
    {
        cpu.regs[rd] = SIGNEXTEND_CAST(val1 / val2, int32_t);
    }
}

void r64::sraw(Cpu& cpu, Decoder decoder)
{
    Cpu::reg_name rd = decoder.rd();
    Cpu::reg_name rs1 = decoder.rs1();
    Cpu::reg_name rs2 = decoder.rs2();

    int32_t val1 = cpu.regs[rs1];
    uint64_t val2 = cpu.regs[rs2] & 0x1f;

    cpu.regs[rd] = SIGNEXTEND_CAST(val1 >> val2, int32_t);
}

void r64::remw(Cpu& cpu, Decoder decoder)
{
    Cpu::reg_name rd = decoder.rd();
    Cpu::reg_name rs1 = decoder.rs1();
    Cpu::reg_name rs2 = decoder.rs2();

    int64_t val1 = static_cast<int32_t>(cpu.regs[rs1]);
    int64_t val2 = cpu.regs[rs2];

    cpu.regs[rd] = SIGNEXTEND_CAST(val1 % val2, int32_t);
}

void r64::remuw(Cpu& cpu, Decoder decoder)
{
    Cpu::reg_name rd = decoder.rd();
    Cpu::reg_name rs1 = decoder.rs1();
    Cpu::reg_name rs2 = decoder.rs2();

    uint64_t val1 = cpu.regs[rs1];
    int64_t val2 = cpu.regs[rs2];

    cpu.regs[rd] = val1 % val2;
}
