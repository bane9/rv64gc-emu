#include "rtypeinsn.hpp"

#include "helper.hpp"
#include <limits>

void rtype::funct3(Cpu& cpu, Decoder decoder)
{
    switch (decoder.funct3())
    {
    case RType::ADDMULSUB:
        switch (decoder.funct7())
        {
        case RType::ADD:
            add(cpu, decoder);
            break;
        case RType::MUL:
            mul(cpu, decoder);
            break;
        case RType::SUB:
            sub(cpu, decoder);
            break;
        default:
            cpu.set_exception(exception::Exception::IllegalInstruction, decoder.insn);
            break;
        }
        break;
    case RType::SLLMULH:
        switch (decoder.funct7())
        {
        case RType::SLL:
            sll(cpu, decoder);
            break;
        case RType::MULH:
            mulh(cpu, decoder);
            break;
        default:
            cpu.set_exception(exception::Exception::IllegalInstruction, decoder.insn);
            break;
        }
        break;
    case RType::SLTMULHSU:
        switch (decoder.funct7())
        {
        case RType::SLT:
            slt(cpu, decoder);
            break;
        case RType::MULHSU:
            mulhsu(cpu, decoder);
            break;
        default:
            cpu.set_exception(exception::Exception::IllegalInstruction, decoder.insn);
            break;
        }
        break;
    case RType::SLTUMULHU:
        switch (decoder.funct7())
        {
        case RType::SLTU:
            sltu(cpu, decoder);
            break;
        case RType::MULHU:
            mulhu(cpu, decoder);
            break;
        default:
            cpu.set_exception(exception::Exception::IllegalInstruction, decoder.insn);
            break;
        }
        break;
    case RType::XORDIV:
        switch (decoder.funct7())
        {
        case RType::XOR:
            xor_(cpu, decoder);
            break;
        case RType::DIV:
            div(cpu, decoder);
            break;
        default:
            cpu.set_exception(exception::Exception::IllegalInstruction, decoder.insn);
            break;
        }
        break;
    case RType::SR:
        switch (decoder.funct7())
        {
        case RType::SRL:
            srl(cpu, decoder);
            break;
        case RType::DIVU:
            divu(cpu, decoder);
            break;
        case RType::SRA:
            sra(cpu, decoder);
            break;
        default:
            cpu.set_exception(exception::Exception::IllegalInstruction, decoder.insn);
            break;
        }
        break;
    case RType::ORREM:
        switch (decoder.funct7())
        {
        case RType::OR:
            or_(cpu, decoder);
            break;
        case RType::REM:
            rem(cpu, decoder);
            break;
        default:
            cpu.set_exception(exception::Exception::IllegalInstruction, decoder.insn);
            break;
        }
        break;
    case RType::ANDREMU:
        switch (decoder.funct7())
        {
        case RType::AND:
            and_(cpu, decoder);
            break;
        case RType::REMU:
            remu(cpu, decoder);
            break;
        default:
            cpu.set_exception(exception::Exception::IllegalInstruction, decoder.insn);
            break;
        }
        break;
    default:
        cpu.set_exception(exception::Exception::IllegalInstruction, decoder.insn);
        break;
    };
}

void rtype::add(Cpu& cpu, Decoder decoder)
{
    Cpu::reg_name rd = decoder.rd();
    Cpu::reg_name rs1 = decoder.rs1();
    Cpu::reg_name rs2 = decoder.rs2();

    int64_t val1 = cpu.regs[rs1];
    int64_t val2 = cpu.regs[rs2];

    cpu.regs[rd] = val1 + val2;
}

void rtype::mul(Cpu& cpu, Decoder decoder)
{
    Cpu::reg_name rd = decoder.rd();
    Cpu::reg_name rs1 = decoder.rs1();
    Cpu::reg_name rs2 = decoder.rs2();

    int64_t val1 = cpu.regs[rs1];
    int64_t val2 = cpu.regs[rs2];

    cpu.regs[rd] = val1 * val2;
}

void rtype::sub(Cpu& cpu, Decoder decoder)
{
    Cpu::reg_name rd = decoder.rd();
    Cpu::reg_name rs1 = decoder.rs1();
    Cpu::reg_name rs2 = decoder.rs2();

    int64_t val1 = cpu.regs[rs1];
    int64_t val2 = cpu.regs[rs2];

    cpu.regs[rd] = val1 - val2;
}

void rtype::sll(Cpu& cpu, Decoder decoder)
{
    Cpu::reg_name rd = decoder.rd();
    Cpu::reg_name rs1 = decoder.rs1();
    Cpu::reg_name rs2 = decoder.rs2();

    int64_t val1 = cpu.regs[rs1];
    int64_t val2 = cpu.regs[rs2];

    cpu.regs[rd] = val1 << val2;
}

void rtype::mulh(Cpu& cpu, Decoder decoder)
{
    Cpu::reg_name rd = decoder.rd();
    Cpu::reg_name rs1 = decoder.rs1();
    Cpu::reg_name rs2 = decoder.rs2();

    __int128_t val1 = static_cast<int64_t>(cpu.regs[rs1]);
    __int128_t val2 = static_cast<int64_t>(cpu.regs[rs2]);

    cpu.regs[rd] = (val1 * val2) >> 64;
}

void rtype::slt(Cpu& cpu, Decoder decoder)
{
    Cpu::reg_name rd = decoder.rd();
    Cpu::reg_name rs1 = decoder.rs1();
    Cpu::reg_name rs2 = decoder.rs2();

    int64_t val1 = cpu.regs[rs1];
    int64_t val2 = cpu.regs[rs2];

    cpu.regs[rd] = val1 < val2;
}

void rtype::mulhsu(Cpu& cpu, Decoder decoder)
{
    Cpu::reg_name rd = decoder.rd();
    Cpu::reg_name rs1 = decoder.rs1();
    Cpu::reg_name rs2 = decoder.rs2();

    __uint128_t val1 = static_cast<__int128_t>(static_cast<int64_t>(cpu.regs[rs1]));
    __uint128_t val2 = cpu.regs[rs2];

    cpu.regs[rd] = (val1 * val2) >> 64;
}

void rtype::sltu(Cpu& cpu, Decoder decoder)
{
    Cpu::reg_name rd = decoder.rd();
    Cpu::reg_name rs1 = decoder.rs1();
    Cpu::reg_name rs2 = decoder.rs2();

    uint64_t val1 = cpu.regs[rs1];
    uint64_t val2 = cpu.regs[rs2];

    cpu.regs[rd] = val1 < val2;
}

void rtype::mulhu(Cpu& cpu, Decoder decoder)
{
    Cpu::reg_name rd = decoder.rd();
    Cpu::reg_name rs1 = decoder.rs1();
    Cpu::reg_name rs2 = decoder.rs2();

    __uint128_t val1 = cpu.regs[rs1];
    __uint128_t val2 = cpu.regs[rs2];

    cpu.regs[rd] = (val1 * val2) >> 64;
}

void rtype::xor_(Cpu& cpu, Decoder decoder)
{
    Cpu::reg_name rd = decoder.rd();
    Cpu::reg_name rs1 = decoder.rs1();
    Cpu::reg_name rs2 = decoder.rs2();

    uint64_t val1 = cpu.regs[rs1];
    uint64_t val2 = cpu.regs[rs2];

    cpu.regs[rd] = val1 ^ val2;
}

void rtype::div(Cpu& cpu, Decoder decoder)
{
    Cpu::reg_name rd = decoder.rd();
    Cpu::reg_name rs1 = decoder.rs1();
    Cpu::reg_name rs2 = decoder.rs2();

    int64_t val1 = cpu.regs[rs1];
    int64_t val2 = cpu.regs[rs2];

    if (val2 == 0)
    {
        cpu.regs[rd] = ~0ULL;
    }
    else if (val1 == std::numeric_limits<int32_t>::min() && val2 == -1)
    {
        cpu.regs[rd] = val1;
    }
    else
    {
        cpu.regs[rd] = val1 / val2;
    }
}

void rtype::srl(Cpu& cpu, Decoder decoder)
{
    Cpu::reg_name rd = decoder.rd();
    Cpu::reg_name rs1 = decoder.rs1();
    Cpu::reg_name rs2 = decoder.rs2();

    uint64_t val1 = cpu.regs[rs1];
    uint64_t val2 = cpu.regs[rs2];

    cpu.regs[rd] = val1 >> val2;
}

void rtype::divu(Cpu& cpu, Decoder decoder)
{
    Cpu::reg_name rd = decoder.rd();
    Cpu::reg_name rs1 = decoder.rs1();
    Cpu::reg_name rs2 = decoder.rs2();

    uint64_t val1 = cpu.regs[rs1];
    uint64_t val2 = cpu.regs[rs2];

    if (val2 == 0)
    {
        cpu.regs[rd] = ~0ULL;
    }
    else
    {
        cpu.regs[rd] = val1 / val2;
    }
}

void rtype::sra(Cpu& cpu, Decoder decoder)
{
    Cpu::reg_name rd = decoder.rd();
    Cpu::reg_name rs1 = decoder.rs1();
    Cpu::reg_name rs2 = decoder.rs2();

    int32_t val1 = cpu.regs[rs1];
    int64_t val2 = cpu.regs[rs2];

    cpu.regs[rd] = val1 >> val2;
}

void rtype::or_(Cpu& cpu, Decoder decoder)
{
    Cpu::reg_name rd = decoder.rd();
    Cpu::reg_name rs1 = decoder.rs1();
    Cpu::reg_name rs2 = decoder.rs2();

    uint64_t val1 = cpu.regs[rs1];
    uint64_t val2 = cpu.regs[rs2];

    cpu.regs[rd] = val1 | val2;
}

void rtype::rem(Cpu& cpu, Decoder decoder)
{
    Cpu::reg_name rd = decoder.rd();
    Cpu::reg_name rs1 = decoder.rs1();
    Cpu::reg_name rs2 = decoder.rs2();

    int64_t val1 = cpu.regs[rs1];
    int64_t val2 = cpu.regs[rs2];

    if (val2 == 0)
    {
        cpu.regs[rd] = val1;
    }
    else if (val1 == std::numeric_limits<int64_t>::min() && val2 == -1)
    {
        cpu.regs[rd] = 0;
    }
    else
    {
        cpu.regs[rd] = val1 % val2;
    }
}

void rtype::and_(Cpu& cpu, Decoder decoder)
{
    Cpu::reg_name rd = decoder.rd();
    Cpu::reg_name rs1 = decoder.rs1();
    Cpu::reg_name rs2 = decoder.rs2();

    uint64_t val1 = cpu.regs[rs1];
    uint64_t val2 = cpu.regs[rs2];

    cpu.regs[rd] = val1 & val2;
}

void rtype::remu(Cpu& cpu, Decoder decoder)
{
    Cpu::reg_name rd = decoder.rd();
    Cpu::reg_name rs1 = decoder.rs1();
    Cpu::reg_name rs2 = decoder.rs2();

    uint64_t val1 = cpu.regs[rs1];
    uint64_t val2 = cpu.regs[rs2];

    if (val2 == 0)
    {
        cpu.regs[rd] = val1;
    }
    else
    {
        cpu.regs[rd] = val1 % val2;
    }
}
