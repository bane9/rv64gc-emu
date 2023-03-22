#pragma once

#include "cpu.hpp"
#include "decoder.hpp"

namespace atomic
{
void funct3(Cpu& cpu, Decoder decoder);

void amoaddw(Cpu& cpu, Decoder decoder);
void amoswapw(Cpu& cpu, Decoder decoder);
void lrw(Cpu& cpu, Decoder decoder);
void scw(Cpu& cpu, Decoder decoder);
void amoxorw(Cpu& cpu, Decoder decoder);
void amoorw(Cpu& cpu, Decoder decoder);
void amoandw(Cpu& cpu, Decoder decoder);
void amominw(Cpu& cpu, Decoder decoder);
void amomaxw(Cpu& cpu, Decoder decoder);
void amominuw(Cpu& cpu, Decoder decoder);
void amomaxuw(Cpu& cpu, Decoder decoder);

void amoaddd(Cpu& cpu, Decoder decoder);
void amoswapd(Cpu& cpu, Decoder decoder);
void lrd(Cpu& cpu, Decoder decoder);
void scd(Cpu& cpu, Decoder decoder);
void amoxord(Cpu& cpu, Decoder decoder);
void amoord(Cpu& cpu, Decoder decoder);
void amoandd(Cpu& cpu, Decoder decoder);
void amomind(Cpu& cpu, Decoder decoder);
void amomaxd(Cpu& cpu, Decoder decoder);
void amominud(Cpu& cpu, Decoder decoder);
void amomaxud(Cpu& cpu, Decoder decoder);
}; // namespace atomic
