#pragma once

#include "cpu.hpp"
#include "decoder.hpp"

namespace i64
{
void funct3(Cpu& cpu, Decoder decoder);

void addiw(Cpu& cpu, Decoder decoder);
void slliw(Cpu& cpu, Decoder decoder);
void srliw(Cpu& cpu, Decoder decoder);
void sraiw(Cpu& cpu, Decoder decoder);
}; // namespace i64