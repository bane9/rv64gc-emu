#pragma once

#include "cpu.hpp"
#include "decoder.hpp"

namespace fence
{
void fence(Cpu& cpu, Decoder decoder);
};

namespace auipc
{
void auipc(Cpu& cpu, Decoder decoder);
};

namespace lui
{
void lui(Cpu& cpu, Decoder decoder);
};

namespace jal
{
void jal(Cpu& cpu, Decoder decoder);
};

namespace jalr
{
void jalr(Cpu& cpu, Decoder decoder);
};
