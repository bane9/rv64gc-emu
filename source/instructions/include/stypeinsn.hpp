#pragma once

#include "cpu.hpp"
#include "decoder.hpp"

namespace stype
{
void funct3(Cpu& cpu, Decoder decoder);

void sb(Cpu& cpu, Decoder decoder);
void sh(Cpu& cpu, Decoder decoder);
void sw(Cpu& cpu, Decoder decoder);
void sd(Cpu& cpu, Decoder decoder);
}; // namespace stype
