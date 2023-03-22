#pragma once

#include "cpu.hpp"
#include "decoder.hpp"

namespace r64
{
void funct3(Cpu& cpu, Decoder decoder);

void addw(Cpu& cpu, Decoder decoder);
void mulw(Cpu& cpu, Decoder decoder);
void subw(Cpu& cpu, Decoder decoder);
void divw(Cpu& cpu, Decoder decoder);
void sllw(Cpu& cpu, Decoder decoder);
void srlw(Cpu& cpu, Decoder decoder);
void divuw(Cpu& cpu, Decoder decoder);
void sraw(Cpu& cpu, Decoder decoder);
void remw(Cpu& cpu, Decoder decoder);
void remuw(Cpu& cpu, Decoder decoder);
}; // namespace r64