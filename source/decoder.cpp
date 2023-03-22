#include "decoder.hpp"
#include "cpu.hpp"
#include "helper.hpp"
#include <array>
#include <fmt/core.h>

Decoder::Decoder(uint32_t insn) : insn(insn)
{
}

uint64_t Decoder::opcode() const
{
    return insn & 0x7fU;
}

OpcodeType Decoder::opcode_type() const
{
    return static_cast<OpcodeType>(opcode());
}

Cpu::reg_name Decoder::rd() const
{
    return static_cast<Cpu::reg_name>((insn >> 7U) & 0x1fU);
}

Cpu::reg_name Decoder::rs1() const
{
    return static_cast<Cpu::reg_name>((insn >> 15U) & 0x1fU);
}

Cpu::reg_name Decoder::rs2() const
{
    return static_cast<Cpu::reg_name>((insn >> 20U) & 0x1fU);
}

Cpu::reg_name Decoder::rs3() const
{
    return static_cast<Cpu::reg_name>((insn & 0xf8000000U) >> 27U);
}

uint64_t Decoder::imm_i() const
{
    return SIGNEXTEND_CAST2(insn & 0xfff00000U, int32_t) >> 20U;
}

uint64_t Decoder::imm_s() const
{
    return (SIGNEXTEND_CAST2(insn & 0xfe000000U, int32_t) >> 20U) | ((insn >> 7) & 0x1fU);
}

uint64_t Decoder::imm_b() const
{
    return (SIGNEXTEND_CAST2(insn & 0x80000000U, int32_t) >> 19U) | ((insn & 0x80U) << 4U) |
           ((insn >> 20U) & 0x7e0U) | ((insn >> 7U) & 0x1eU);
}

uint64_t Decoder::imm_u() const
{
    return SIGNEXTEND_CAST(insn & 0xfffff999U, int32_t);
}

uint64_t Decoder::imm_j() const
{
    return (SIGNEXTEND_CAST2(insn & 0x80000000U, int32_t) >> 11U) | (insn & 0xff000U) |
           ((insn >> 9U) & 0x800U) | ((insn >> 20U) & 0x7feU);
}

uint64_t Decoder::funct2() const
{
    return (insn & 0x03000000U) >> 25U;
}

uint64_t Decoder::funct3() const
{
    return (insn >> 12U) & 0x7U;
}

uint64_t Decoder::funct5() const
{
    return (funct7() & 0x7cU) >> 2U;
}

uint64_t Decoder::funct7() const
{
    return (insn >> 25U) & 0x7fU;
}

uint64_t Decoder::fl_offset() const
{
    return SIGNEXTEND_CAST2(insn, int32_t) >> 20;
}

uint64_t Decoder::fs_offset() const
{
    return (fl_offset() & 0xfe0U) | ((insn >> 7U) & 0x1fU);
}

uint32_t Decoder::shamt() const
{
    return imm_i() & 0x3fU;
}

uint64_t Decoder::csr() const
{
    return (insn & 0xfff00000U) >> 20U;
}

uint64_t Decoder::compressed_opcode() const
{
    return insn & 0x03U;
}

uint64_t Decoder::insn_size() const
{
    switch (static_cast<OpcodeType>(compressed_opcode()))
    {
    case OpcodeType::COMPRESSED_QUANDRANT0:
    case OpcodeType::COMPRESSED_QUANDRANT1:
    case OpcodeType::COMPRESSED_QUANDRANT2:
        return 2;
    default:
        return 4;
    }
}

uint64_t Decoder::compressed_rd() const
{
    return ((insn >> 2U) & 0x07U) + 8U;
}

uint64_t Decoder::compressed_rs1() const
{
    return ((insn >> 7U) & 0x07U) + 8U;
}

uint64_t Decoder::compressed_rs2() const
{
    return ((insn >> 2U) & 0x07U) + 8U;
}

uint64_t Decoder::compressed_shamt() const
{
    return ((insn >> 7U) & 0x20U) | ((insn >> 2U) & 0x1fU);
}

uint64_t Decoder::compressed_funct3() const
{
    return (insn >> 13U) & 0x07U;
}

uint64_t Decoder::compressed_funct2() const
{
    return (insn >> 10U) & 0x3U;
}

FPURoundigMode::Mode Decoder::fp_rounding_mode() const
{
    return static_cast<FPURoundigMode::Mode>((insn >> 12U) & 0x3U);
}

void Decoder::dump(std::ostream& stream) const
{
    const char* opcode_str = "unknown";

    switch (opcode_type())
    {
    case OpcodeType::LOAD:
        opcode_str = "LOAD";
        break;
    case OpcodeType::FENCE:
        opcode_str = "FENCE";
        break;
    case OpcodeType::AUIPC:
        opcode_str = "AUIPC";
        break;
    case OpcodeType::LUI:
        opcode_str = "LUI";
        break;
    case OpcodeType::JALR:
        opcode_str = "JALR";
        break;
    case OpcodeType::JAL:
        opcode_str = "JAL";
        break;
    case OpcodeType::I:
        opcode_str = "I";
        break;
    case OpcodeType::S:
        opcode_str = "S";
        break;
    case OpcodeType::R:
        opcode_str = "R";
        break;
    case OpcodeType::B:
        opcode_str = "B";
        break;
    case OpcodeType::FL:
        opcode_str = "FL";
        break;
    case OpcodeType::FS:
        opcode_str = "FS";
        break;
    case OpcodeType::FMADD:
        opcode_str = "FMADD";
        break;
    case OpcodeType::FMSUB:
        opcode_str = "FMSUB";
        break;
    case OpcodeType::FNMADD:
        opcode_str = "FNMADD";
        break;
    case OpcodeType::FNMSUB:
        opcode_str = "FNMSUB";
        break;
    case OpcodeType::FOTHER:
        opcode_str = "FOTHER";
        break;
    case OpcodeType::I64:
        opcode_str = "I64";
        break;
    case OpcodeType::R64:
        opcode_str = "R64";
        break;
    case OpcodeType::CSR:
        opcode_str = "CSR";
        break;
    case OpcodeType::ATOMIC:
        opcode_str = "ATOMIC";
        break;
    default:
        break;
    }

    if (insn_size() == 2)
    {
        opcode_str = "Compressed";
    }

    stream << fmt::format("insn: {:0>4x}\n", insn);
    stream << fmt::format("insn size: {}\n", insn_size());
    stream << fmt::format("opcode: {} (0x{:0>2x})\n", opcode_str, opcode());
    stream << fmt::format("compressed opcode: 0x{:x}\n", compressed_opcode());
    stream << fmt::format("source reg 1: {} (0x{:0>2x})\n", Cpu::reg_name_abi_str[rs1()], rs1());
    stream << fmt::format("source reg 2: {} (0x{:0>2x})\n", Cpu::reg_name_abi_str[rs2()], rs2());
    stream << fmt::format("source reg 3: {} (0x{:0>2x})\n", Cpu::reg_name_abi_str[rs3()], rs3());
    stream << fmt::format("destination reg: {} (0x{:0>2x})\n", Cpu::reg_name_abi_str[rd()], rd());

    stream << fmt::format("imm_i: {:0>4x}\n", imm_i());
    stream << fmt::format("imm_s: {:0>4x}\n", imm_s());
    stream << fmt::format("imm_b: {:0>4x}\n", imm_b());
    stream << fmt::format("imm_u: {:0>4x}\n", imm_u());
    stream << fmt::format("imm_j: {:0>4x}\n", imm_j());

    stream << fmt::format("funct2: {:0>4x}\n", funct2());
    stream << fmt::format("funct3: {:0>4x}\n", funct3());
    stream << fmt::format("funct5: {:0>4x}\n", funct5());
    stream << fmt::format("funct7: {:0>4x}\n", funct7());

    stream << fmt::format("compressed funct2: {:0>2x}\n", compressed_funct2());
    stream << fmt::format("compressed funct3: {:0>2x}\n", compressed_funct3());

    stream << fmt::format("shamt: {:0>4x}\n", shamt());
    stream << fmt::format("csr: {:0>4x}", csr());
}