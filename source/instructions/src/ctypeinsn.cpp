#include "ctypeinsn.hpp"

#include "helper.hpp"

void ctype::quadrant0(Cpu& cpu, Decoder decoder)
{
    switch (decoder.compressed_funct3())
    {
    case Q0::ADDI4SPN:
        addi4spn(cpu, decoder);
        break;
    case Q0::FLD:
        fld(cpu, decoder);
        break;
    case Q0::LW:
        lw(cpu, decoder);
        break;
    case Q0::LD:
        ld(cpu, decoder);
        break;
    case Q0::RESERVED:
        reserved(cpu, decoder);
        break;
    case Q0::FSD:
        fsd(cpu, decoder);
        break;
    case Q0::SW:
        sw(cpu, decoder);
        break;
    case Q0::SD:
        sd(cpu, decoder);
        break;
    default:
        cpu.set_exception(exception::Exception::IllegalInstruction, decoder.insn);
        break;
    };
}

static void q1op4(Cpu& cpu, Decoder decoder)
{
    using namespace ctype;

    switch (decoder.compressed_funct2())
    {
    case Q1::OP4::funct2::SRLI:
        srli(cpu, decoder);
        break;
    case Q1::OP4::funct2::SRAI:
        srai(cpu, decoder);
        break;
    case Q1::OP4::funct2::ANDI:
        andi(cpu, decoder);
        break;
    case Q1::OP4::funct2::OP3: {
        uint32_t val1 = (decoder.insn >> 12U) & 0x01U;
        uint32_t val2 = (decoder.insn >> 5U) & 0x03U;

        if (val1 == 0)
        {
            switch (val2)
            {
            case 0:
                sub(cpu, decoder);
                break;
            case 1:
                xor_(cpu, decoder);
                break;
            case 2:
                or_(cpu, decoder);
                break;
            case 3:
                and_(cpu, decoder);
                break;
            default:
                break;
            }
        }
        else
        {
            switch (val2)
            {
            case 0:
                subw(cpu, decoder);
                break;
            case 1:
                addw(cpu, decoder);
                break;
            default:
                cpu.set_exception(exception::Exception::IllegalInstruction, decoder.insn);
                break;
            }
        }
    }
    break;
    default:
        cpu.set_exception(exception::Exception::IllegalInstruction, decoder.insn);
        break;
    }
}

void ctype::quadrant1(Cpu& cpu, Decoder decoder)
{
    switch (decoder.compressed_funct3())
    {
    case Q1::ADDI:
        addi(cpu, decoder);
        break;
    case Q1::ADDIW:
        addiw(cpu, decoder);
        break;
    case Q1::LI:
        li(cpu, decoder);
        break;
    case Q1::OP03:
        switch (static_cast<Q1::OP03fn>(decoder.rd()))
        {
        case Q1::OP03fn::NOP:
            break;
        case Q1::OP03fn::ADDI16SP:
            addi16sp(cpu, decoder);
            break;
        default:
            lui(cpu, decoder);
            break;
        }
        break;
    case Q1::OP04:
        q1op4(cpu, decoder);
        break;
    case Q1::J:
        j(cpu, decoder);
        break;
    case Q1::BEQZ:
        beqz(cpu, decoder);
        break;
    case Q1::BNEZ:
        bnez(cpu, decoder);
        break;
    default:
        cpu.set_exception(exception::Exception::IllegalInstruction, decoder.insn);
        break;
    };
}

void ctype::quadrant2(Cpu& cpu, Decoder decoder)
{
    switch (decoder.compressed_funct3())
    {
    case Q2::SLLI:
        slli(cpu, decoder);
        break;
    case Q2::FLDSP:
        fldsp(cpu, decoder);
        break;
    case Q2::LWSP:
        lwsp(cpu, decoder);
        break;
    case Q2::LDSP:
        ldsp(cpu, decoder);
        break;
    case Q2::OP4: {
        uint32_t val1 = (decoder.insn >> 12U) & 0x01U;
        uint32_t val2 = (decoder.insn >> 2U) & 0x1fU;

        if (val1 == 0)
        {
            if (val2 == 0)
            {
                jr(cpu, decoder);
            }
            else
            {
                mv(cpu, decoder);
            }
        }
        else
        {
            if (val2 == 0)
            {
                jalr(cpu, decoder);
            }
            else
            {
                add(cpu, decoder);
            }
        }
    }
    break;
    case Q2::FSDSP:
        fsdsp(cpu, decoder);
        break;
    case Q2::SWSP:
        swsp(cpu, decoder);
        break;
    case Q2::SDSP:
        sdsp(cpu, decoder);
        break;
    default:
        cpu.set_exception(exception::Exception::IllegalInstruction, decoder.insn);
        break;
    }
}

void ctype::addi4spn(Cpu& cpu, Decoder decoder)
{
    uint64_t rd = decoder.compressed_rd();

    uint64_t imm = ((decoder.insn >> 1U) & 0x3c0U) | ((decoder.insn >> 7U) & 0x30U) |
                   ((decoder.insn >> 2U) & 0x08U) | ((decoder.insn >> 4U) & 0x04U);

    uint64_t val = cpu.regs[Cpu::reg_abi_name::sp] + imm;

    cpu.regs[rd] = val;
}

void ctype::fld(Cpu& cpu, Decoder decoder)
{
    uint64_t rd = decoder.compressed_rd();
    uint64_t rs1 = decoder.compressed_rs1();

    uint64_t offset = ((decoder.insn << 1U) & 0xc0U) | ((decoder.insn >> 7U) & 0x38U);

    uint64_t address = cpu.regs[rs1] + offset;

    uint64_t bit_val = cpu.mmu.load(address, 64);

    cpu.fregs[rd] = bit_val;
}

void ctype::lw(Cpu& cpu, Decoder decoder)
{
    uint64_t rd = decoder.compressed_rd();
    uint64_t rs1 = decoder.compressed_rs1();

    uint64_t offset = ((decoder.insn << 1U) & 0x40u) | ((decoder.insn >> 7U) & 0x38U) |
                      ((decoder.insn >> 4U) & 0x04U);

    uint64_t address = cpu.regs[rs1] + offset;
    uint32_t val = cpu.mmu.load(address, 32);

    cpu.regs[rd] = SIGNEXTEND_CAST(val, int32_t);
}

void ctype::ld(Cpu& cpu, Decoder decoder)
{
    uint64_t rd = decoder.compressed_rd();
    uint64_t rs1 = decoder.compressed_rs1();

    uint64_t offset = ((decoder.insn << 1U) & 0xc0U) | ((decoder.insn >> 7U) & 0x38U);

    uint64_t address = cpu.regs[rs1] + offset;

    cpu.regs[rd] = cpu.mmu.load(address, 64);
}

void ctype::reserved(Cpu& cpu, Decoder decoder)
{
    cpu.set_exception(exception::Exception::IllegalInstruction, decoder.insn);
}

void ctype::fsd(Cpu& cpu, Decoder decoder)
{
    uint64_t rd = decoder.compressed_rd();
    uint64_t rs1 = decoder.compressed_rs1();

    uint64_t offset = ((decoder.insn << 1U) & 0xc0U) | ((decoder.insn >> 7U) & 0x38U);

    uint64_t address = cpu.regs[rs1] + offset;

    uint64_t rs1_bits = cpu.fregs[rs1].get_u64();

    cpu.mmu.store(address, rs1_bits, 64);
}

void ctype::sw(Cpu& cpu, Decoder decoder)
{
    uint64_t rs1 = decoder.compressed_rs1();
    uint64_t rs2 = decoder.compressed_rs2();

    uint64_t offset = ((decoder.insn << 1U) & 0x40U) | ((decoder.insn >> 7U) & 0x38U) |
                      ((decoder.insn >> 4U) & 0x04U);

    uint64_t address = cpu.regs[rs1] + offset;
    uint32_t val = cpu.regs[rs2];

    cpu.mmu.store(address, val, 32);
}

void ctype::sd(Cpu& cpu, Decoder decoder)
{
    uint64_t rs1 = decoder.compressed_rs1();
    uint64_t rs2 = decoder.compressed_rs2();

    uint64_t offset = ((decoder.insn << 1U) & 0xc0U) | ((decoder.insn >> 7U) & 0x38U);

    uint64_t address = cpu.regs[rs1] + offset;
    uint64_t val = cpu.regs[rs2];

    cpu.mmu.store(address, val, 64);
}

void ctype::addi(Cpu& cpu, Decoder decoder)
{
    uint64_t rd = decoder.rd();

    uint64_t imm = ((decoder.insn >> 7U) & 0x20U) | ((decoder.insn >> 2U) & 0x1fU);

    if ((imm & 0x20) != 0)
    {
        imm = SIGNEXTEND_CAST(imm | 0xc0U, int8_t);
    }

    cpu.regs[rd] += imm;
}

void ctype::addiw(Cpu& cpu, Decoder decoder)
{
    uint64_t rd = decoder.rd();

    uint64_t imm = ((decoder.insn >> 7U) & 0x20U) | ((decoder.insn >> 2U) & 0x1fU);

    if ((imm & 0x20) != 0)
    {
        imm = SIGNEXTEND_CAST(imm | 0xc0U, int8_t);
    }

    cpu.regs[rd] = SIGNEXTEND_CAST(cpu.regs[rd] + imm, int32_t);
}

void ctype::li(Cpu& cpu, Decoder decoder)
{
    uint64_t rd = decoder.rd();

    uint64_t imm = ((decoder.insn >> 7U) & 0x20U) | ((decoder.insn >> 2U) & 0x1fU);

    if ((imm & 0x20) != 0)
    {
        imm = SIGNEXTEND_CAST(imm | 0xc0U, int8_t);
    }

    cpu.regs[rd] = imm;
}

void ctype::addi16sp(Cpu& cpu, Decoder decoder)
{
    uint64_t imm = ((decoder.insn >> 3U) & 0x200U) | ((decoder.insn >> 2U) & 0x10U) |
                   ((decoder.insn << 1U) & 0x40U) | ((decoder.insn << 4U) & 0x180U) |
                   ((decoder.insn << 3U) & 0x20U);

    if ((imm & 0x200) != 0)
    {
        imm = SIGNEXTEND_CAST(imm | 0xfc00U, int16_t);
    }

    cpu.regs[Cpu::reg_abi_name::sp] += imm;
}

void ctype::lui(Cpu& cpu, Decoder decoder)
{
    uint64_t rd = decoder.rd();

    uint64_t imm = ((decoder.insn << 5U) & 0x20000U) | ((decoder.insn << 10U) & 0x1f000U);

    if ((imm & 0x20000U) != 0)
    {
        imm = SIGNEXTEND_CAST(imm | 0xfffc0000U, int32_t);
    }

    cpu.regs[rd] = imm;
}

void ctype::srli(Cpu& cpu, Decoder decoder)
{
    uint64_t rd = decoder.compressed_rs1();
    uint64_t shamt = decoder.compressed_shamt();

    cpu.regs[rd] >>= shamt;
}

void ctype::srai(Cpu& cpu, Decoder decoder)
{
    uint64_t rd = decoder.compressed_rs1();
    uint64_t shamt = decoder.compressed_shamt();

    cpu.regs[rd] = static_cast<int64_t>(cpu.regs[rd]) >> shamt;
}

void ctype::andi(Cpu& cpu, Decoder decoder)
{
    uint64_t rd = decoder.compressed_rs1();

    uint64_t imm = ((decoder.insn >> 7U) & 0x20U) | ((decoder.insn >> 2U) & 0x1fU);

    if ((imm & 0x20U) != 0)
    {
        imm = SIGNEXTEND_CAST(imm | 0xc0U, int8_t);
    }

    cpu.regs[rd] &= imm;
}

void ctype::sub(Cpu& cpu, Decoder decoder)
{
    uint64_t rd = decoder.compressed_rs1();
    uint64_t rs2 = decoder.compressed_rs2();

    cpu.regs[rd] -= cpu.regs[rs2];
}

void ctype::xor_(Cpu& cpu, Decoder decoder)
{
    uint64_t rd = decoder.compressed_rs1();
    uint64_t rs2 = decoder.compressed_rs2();

    cpu.regs[rd] ^= cpu.regs[rs2];
}

void ctype::or_(Cpu& cpu, Decoder decoder)
{
    uint64_t rd = decoder.compressed_rs1();
    uint64_t rs2 = decoder.compressed_rs2();

    cpu.regs[rd] |= cpu.regs[rs2];
}

void ctype::and_(Cpu& cpu, Decoder decoder)
{
    uint64_t rd = decoder.compressed_rs1();
    uint64_t rs2 = decoder.compressed_rs2();

    cpu.regs[rd] &= cpu.regs[rs2];
}

void ctype::subw(Cpu& cpu, Decoder decoder)
{
    uint64_t rd = decoder.compressed_rs1();
    uint64_t rs2 = decoder.compressed_rs2();

    cpu.regs[rd] = SIGNEXTEND_CAST(cpu.regs[rd] - cpu.regs[rs2], int32_t);
}

void ctype::addw(Cpu& cpu, Decoder decoder)
{
    uint64_t rd = decoder.compressed_rs1();
    uint64_t rs2 = decoder.compressed_rs2();

    cpu.regs[rd] = SIGNEXTEND_CAST(cpu.regs[rd] + cpu.regs[rs2], int32_t);
}

static inline uint64_t bit_mask(uint64_t count)
{
    return (1ULL << count) - 1;
}

void ctype::j(Cpu& cpu, Decoder decoder)
{
    uint64_t imm = ((decoder.insn >> 1U) & 0x800U) | ((decoder.insn << 2U) & 0x400U) |
                   ((decoder.insn >> 1U) & 0x300U) | ((decoder.insn << 1U) & 0x80U) |
                   ((decoder.insn >> 1U) & 0x40U) | ((decoder.insn << 3U) & 0x20U) |
                   ((decoder.insn >> 7U) & 0x10U) | ((decoder.insn >> 2U) & 0xeU);

    if ((imm & 0x800U) != 0)
    {
        imm = SIGNEXTEND_CAST(imm | 0xf000U, int16_t);
    }

    cpu.pc += imm - 2;
}

void ctype::beqz(Cpu& cpu, Decoder decoder)
{
    uint64_t rs1 = decoder.compressed_rs1();

    uint64_t imm = ((decoder.insn >> 4U) & 0x100U) | ((decoder.insn << 1U) & 0xc0U) |
                   ((decoder.insn << 3U) & 0x20U) | ((decoder.insn >> 7U) & 0x18U) |
                   ((decoder.insn >> 2U) & 0x6U);

    if ((imm & 0x100) != 0)
    {
        imm = SIGNEXTEND_CAST(imm | 0xfe00U, int16_t);
    }

    if (cpu.regs[rs1] == 0)
    {
        cpu.pc += imm - 2;
    }
}

void ctype::bnez(Cpu& cpu, Decoder decoder)
{
    uint64_t rs1 = decoder.compressed_rs1();

    uint64_t imm = ((decoder.insn >> 4U) & 0x100U) | ((decoder.insn << 1U) & 0xc0U) |
                   ((decoder.insn << 3U) & 0x20U) | ((decoder.insn >> 7U) & 0x18U) |
                   ((decoder.insn >> 2U) & 0x6U);

    if ((imm & 0x100) != 0)
    {
        imm = SIGNEXTEND_CAST(imm | 0xfe00U, int16_t);
    }

    if (cpu.regs[rs1] != 0)
    {
        cpu.pc += imm - 2;
    }
}

void ctype::slli(Cpu& cpu, Decoder decoder)
{
    uint64_t rd = decoder.rd();
    uint64_t shamt = decoder.compressed_shamt();

    cpu.regs[rd] <<= shamt;
}

void ctype::fldsp(Cpu& cpu, Decoder decoder)
{
    uint64_t rd = decoder.rd();

    uint64_t offset = ((decoder.insn << 4U) & 0x1c0U) | ((decoder.insn >> 7U) & 0x20U) |
                      ((decoder.insn >> 2U) & 0x18U);

    uint64_t val = cpu.mmu.load(cpu.regs[Cpu::reg_abi_name::sp] + offset, 64);

    cpu.fregs[rd] = val;
}

void ctype::lwsp(Cpu& cpu, Decoder decoder)
{
    uint64_t rd = decoder.rd();

    uint64_t offset = ((decoder.insn << 4U) & 0xc0U) | ((decoder.insn >> 7U) & 0x20U) |
                      ((decoder.insn >> 2U) & 0x1cU);

    uint32_t val = cpu.mmu.load(cpu.regs[Cpu::reg_abi_name::sp] + offset, 32);

    cpu.regs[rd] = SIGNEXTEND_CAST(val, int32_t);
}

void ctype::ldsp(Cpu& cpu, Decoder decoder)
{
    uint64_t rd = decoder.rd();

    uint64_t offset = ((decoder.insn << 4U) & 0x1c0U) | ((decoder.insn >> 7U) & 0x20U) |
                      ((decoder.insn >> 2U) & 0x18U);

    uint64_t val = cpu.mmu.load(cpu.regs[Cpu::reg_abi_name::sp] + offset, 64);

    cpu.regs[rd] = val;
}

void ctype::jr(Cpu& cpu, Decoder decoder)
{
    uint64_t rs1 = decoder.rd();

    cpu.pc = cpu.regs[rs1] - 2;
}

void ctype::mv(Cpu& cpu, Decoder decoder)
{
    uint64_t rd = decoder.rd();
    uint64_t rs1 = (decoder.insn >> 2U) & 0x1fU;

    cpu.regs[rd] = cpu.regs[rs1];
}

void ctype::ebreak(Cpu& cpu, Decoder decoder)
{
    cpu.set_exception(exception::Exception::Breakpoint);
}

void ctype::jalr(Cpu& cpu, Decoder decoder)
{
    uint64_t rs1 = decoder.rd();

    uint64_t temp = cpu.pc + 2;

    cpu.pc = cpu.regs[rs1] - 2;

    cpu.regs[Cpu::reg_abi_name::ra] = temp;
}

void ctype::add(Cpu& cpu, Decoder decoder)
{
    uint64_t rd = decoder.rd();
    uint64_t rs1 = (decoder.insn >> 2U) & 0x1fU;

    cpu.regs[rd] += cpu.regs[rs1];
}

void ctype::fsdsp(Cpu& cpu, Decoder decoder)
{
    uint64_t rs1 = (decoder.insn >> 2U) & 0x1fU;

    uint64_t offset = ((decoder.insn >> 1U) & 0x1c0U) | ((decoder.insn >> 7U) & 0x38U);

    uint64_t address = cpu.regs[Cpu::reg_abi_name::sp] + offset;

    uint64_t rs1_bits = cpu.fregs[rs1];

    cpu.mmu.store(address, rs1_bits, 64);
}

void ctype::swsp(Cpu& cpu, Decoder decoder)
{
    uint64_t rs1 = (decoder.insn >> 2U) & 0x1fU;

    uint64_t offset = ((decoder.insn >> 1U) & 0xc0U) | ((decoder.insn >> 7U) & 0x3cU);

    uint64_t address = cpu.regs[Cpu::reg_abi_name::sp] + offset;

    uint32_t val = cpu.regs[rs1];

    cpu.mmu.store(address, val, 32);
}

void ctype::sdsp(Cpu& cpu, Decoder decoder)
{
    uint64_t rs1 = (decoder.insn >> 2U) & 0x1fU;

    uint64_t offset = ((decoder.insn >> 1U) & 0x1c0U) | ((decoder.insn >> 7U) & 0x38U);

    uint64_t address = cpu.regs[Cpu::reg_abi_name::sp] + offset;

    uint64_t val = cpu.regs[rs1];

    cpu.mmu.store(address, val, 64);
}
