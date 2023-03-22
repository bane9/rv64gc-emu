#pragma once

#include "cpu.hpp"
#include "decoder.hpp"

namespace csr
{
void init_handler_array();

void funct3(Cpu& cpu, Decoder decoder);

void environment(Cpu& cpu, Decoder decoder);
void ecall(Cpu& cpu, Decoder decoder);
void ebreak(Cpu& cpu, Decoder decoder);
void uret(Cpu& cpu, Decoder decoder);
void sret(Cpu& cpu, Decoder decoder);
void mret(Cpu& cpu, Decoder decoder);
void wfi(Cpu& cpu, Decoder decoder);
void sfencevma(Cpu& cpu, Decoder decoder);
void hfencevma(Cpu& cpu, Decoder decoder);
void hfencegvma(Cpu& cpu, Decoder decoder);
void csrw(Cpu& cpu, Decoder decoder);
void csrs(Cpu& cpu, Decoder decoder);
void csrc(Cpu& cpu, Decoder decoder);
void csrwi(Cpu& cpu, Decoder decoder);
void csrsi(Cpu& cpu, Decoder decoder);
void csrci(Cpu& cpu, Decoder decoder);
}; // namespace csr