#pragma once

#include "cpu.hpp"
#include "decoder.hpp"

namespace itype
{
void funct3(Cpu& cpu, Decoder decoder);

void addi(Cpu& cpu, Decoder decoder);
void slli(Cpu& cpu, Decoder decoder);
void slti(Cpu& cpu, Decoder decoder);
void sltiu(Cpu& cpu, Decoder decoder);
void xori(Cpu& cpu, Decoder decoder);
void ori(Cpu& cpu, Decoder decoder);
void andi(Cpu& cpu, Decoder decoder);

void srli(Cpu& cpu, Decoder decoder);
void srai(Cpu& cpu, Decoder decoder);
}; // namespace itype