#pragma once

#include "cpu.hpp"
#include "decoder.hpp"

namespace ctype
{
struct Q0
{
    enum funct3 : uint64_t
    {
        ADDI4SPN = 0x00,
        FLD = 0x01,
        LW = 0x02,
        LD = 0x03,
        RESERVED = 0x04,
        FSD = 0x05,
        SW = 0x06,
        SD = 0x07,
    };
};

struct Q1
{
    enum funct3 : uint64_t
    {
        ADDI = 0x00,
        ADDIW = 0x01,
        LI = 0x02,

        OP03 = 0x03,
        OP04 = 0x04,

        J = 0x05,
        BEQZ = 0x06,
        BNEZ = 0x07,
    };

    enum OP03fn : uint64_t
    {
        NOP = 0x00,
        ADDI16SP = 0x02,
        LUI
    };

    struct OP4
    {
        enum funct2 : uint64_t
        {
            SRLI = 0x00,
            SRAI = 0x01,
            ANDI = 0x02,

            OP3 = 0x03,
        };
    };
};

struct Q2
{
    enum funct3 : uint64_t
    {
        SLLI = 0x00,
        FLDSP = 0x01,
        LWSP = 0x02,
        LDSP = 0x03,

        OP4 = 0x04,

        FSDSP = 0x05,
        SWSP = 0x06,
        SDSP = 0x07,
    };
};

void quadrant0(Cpu& cpu, Decoder decoder);
void quadrant1(Cpu& cpu, Decoder decoder);
void quadrant2(Cpu& cpu, Decoder decoder);

// Q0

void addi4spn(Cpu& cpu, Decoder decoder);
void fld(Cpu& cpu, Decoder decoder);
void lw(Cpu& cpu, Decoder decoder);
void ld(Cpu& cpu, Decoder decoder);
void reserved(Cpu& cpu, Decoder decoder);
void fsd(Cpu& cpu, Decoder decoder);
void sw(Cpu& cpu, Decoder decoder);
void sd(Cpu& cpu, Decoder decoder);

// Q1

void addi(Cpu& cpu, Decoder decoder);
void addiw(Cpu& cpu, Decoder decoder);
void li(Cpu& cpu, Decoder decoder);
void addi16sp(Cpu& cpu, Decoder decoder);
void lui(Cpu& cpu, Decoder decoder);
void srli(Cpu& cpu, Decoder decoder);
void srai(Cpu& cpu, Decoder decoder);
void andi(Cpu& cpu, Decoder decoder);
void sub(Cpu& cpu, Decoder decoder);
void xor_(Cpu& cpu, Decoder decoder);
void or_(Cpu& cpu, Decoder decoder);
void and_(Cpu& cpu, Decoder decoder);
void subw(Cpu& cpu, Decoder decoder);
void addw(Cpu& cpu, Decoder decoder);
void j(Cpu& cpu, Decoder decoder);
void beqz(Cpu& cpu, Decoder decoder);
void bnez(Cpu& cpu, Decoder decoder);

// Q2

void slli(Cpu& cpu, Decoder decoder);
void fldsp(Cpu& cpu, Decoder decoder);
void lwsp(Cpu& cpu, Decoder decoder);
void ldsp(Cpu& cpu, Decoder decoder);
void jr(Cpu& cpu, Decoder decoder);
void mv(Cpu& cpu, Decoder decoder);
void ebreak(Cpu& cpu, Decoder decoder);
void jalr(Cpu& cpu, Decoder decoder);
void add(Cpu& cpu, Decoder decoder);
void fsdsp(Cpu& cpu, Decoder decoder);
void swsp(Cpu& cpu, Decoder decoder);
void sdsp(Cpu& cpu, Decoder decoder);
}; // namespace ctype