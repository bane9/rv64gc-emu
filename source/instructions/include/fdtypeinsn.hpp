#pragma once

#include "cpu.hpp"
#include "decoder.hpp"

namespace fdtype
{
struct FClass
{
    enum FClassValue : uint64_t
    {
        INF = 1 << 0,
        NORMAL = 1 << 1,
        SUBNORMAL = 1 << 2,
        NEG_ZERO = 1 << 3,
        POS_ZERO = 1 << 4,
        POS_SUBNORMAL = 1 << 5,
        POS_NORMAL = 1 << 6,
        POS_INF = 1 << 7,
        NAN_SIG = 1 << 8,
        NAN_QUIET = 1 << 9,
    };
};

void fs(Cpu& cpu, Decoder decoder);
void fl(Cpu& cpu, Decoder decoder);
void fmadd(Cpu& cpu, Decoder decoder);
void fmsub(Cpu& cpu, Decoder decoder);
void fnmadd(Cpu& cpu, Decoder decoder);
void fnmsub(Cpu& cpu, Decoder decoder);
void fother(Cpu& cpu, Decoder decoder);
} // namespace fdtype
