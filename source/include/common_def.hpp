#pragma once
#include <cstdint>
// File created to avoid circular include issues

struct FPURoundigMode
{
    enum Mode : uint64_t
    {
        RoundToNearest = 0x00,
        RoundToZero = 0x01,
        RoundDown = 0x02,
        RoundUp = 0x03,
        RoundNearestMaxMagnitude = 0x04,
        RoundDynamic = 0x05,

        Mask = RoundToNearest | RoundToZero | RoundDown | RoundUp | RoundNearestMaxMagnitude |
               RoundDynamic
    };
};

namespace cpu
{
enum Mode : uint64_t
{
    User = 0,
    Supervisor = 1,
    Machine = 3,

    Invalid = 0xff,
};
}; // namespace cpu