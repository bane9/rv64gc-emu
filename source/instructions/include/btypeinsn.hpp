#pragma once

#include "cpu.hpp"
#include "decoder.hpp"

namespace btype
{
void funct3(Cpu& cpu, Decoder decoder);

void beq(Cpu& cpu, Decoder decoder);
void bne(Cpu& cpu, Decoder decoder);
void blt(Cpu& cpu, Decoder decoder);
void bge(Cpu& cpu, Decoder decoder);
void bltu(Cpu& cpu, Decoder decoder);
void bgeu(Cpu& cpu, Decoder decoder);
}; // namespace btype
