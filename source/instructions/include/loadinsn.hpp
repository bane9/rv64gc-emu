#pragma once

#include "cpu.hpp"
#include "decoder.hpp"

namespace load
{

void funct3(Cpu& cpu, Decoder insn);

void lb(Cpu& cpu, Decoder insn);
void lh(Cpu& cpu, Decoder insn);
void lw(Cpu& cpu, Decoder insn);
void ld(Cpu& cpu, Decoder insn);
void lbu(Cpu& cpu, Decoder insn);
void lhu(Cpu& cpu, Decoder insn);
void lwu(Cpu& cpu, Decoder insn);

}; // namespace load
