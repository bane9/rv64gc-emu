#pragma once

#include "cpu.hpp"
#include "decoder.hpp"

namespace rtype
{
void funct3(Cpu& cpu, Decoder decoder);

void add(Cpu& cpu, Decoder decoder);
void mul(Cpu& cpu, Decoder decoder);
void sub(Cpu& cpu, Decoder decoder);
void sll(Cpu& cpu, Decoder decoder);
void mulh(Cpu& cpu, Decoder decoder);
void slt(Cpu& cpu, Decoder decoder);
void mulhsu(Cpu& cpu, Decoder decoder);
void sltu(Cpu& cpu, Decoder decoder);
void mulhu(Cpu& cpu, Decoder decoder);
void xor_(Cpu& cpu, Decoder decoder);
void div(Cpu& cpu, Decoder decoder);
void srl(Cpu& cpu, Decoder decoder);
void divu(Cpu& cpu, Decoder decoder);
void sra(Cpu& cpu, Decoder decoder);
void or_(Cpu& cpu, Decoder decoder);
void rem(Cpu& cpu, Decoder decoder);
void and_(Cpu& cpu, Decoder decoder);
void remu(Cpu& cpu, Decoder decoder);
} // namespace rtype
