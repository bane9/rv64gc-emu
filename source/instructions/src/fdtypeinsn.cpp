#include "fdtypeinsn.hpp"

#include "helper.hpp"
#include <cfenv>
#include <cfloat>
#include <cmath>
#include <cstdlib>
#include <limits>
#include <type_traits>
#include <utility>

namespace fdimpl
{
template <typename T>
using f_to_uint_t = typename std::conditional<sizeof(T) == 4, uint32_t, uint64_t>::type;

template <typename T> T get_canonical_nan()
{
    f_to_uint_t<T> nan_raw = 0;

    if constexpr (std::is_same_v<T, float>)
    {
        nan_raw = 0x7fc00000U;
    }
    else
    {
        nan_raw = 0x7ff8000000000000ULL;
    }

    return helper::bit_cast<T>(nan_raw);
}

template <typename T> bool is_cnan(T valf)
{
    f_to_uint_t<T> nan_raw = 0;
    f_to_uint_t<T> val = helper::bit_cast<f_to_uint_t<T>>(valf);

    if constexpr (std::is_same_v<T, float>)
    {
        nan_raw = 0x7fc00000U;
    }
    else
    {
        nan_raw = 0x7ff8000000000000ULL;
    }

    return nan_raw == val;
}

template <typename T> bool is_snan(T valf)
{
    f_to_uint_t<T> snan_raw = 0;
    f_to_uint_t<T> val = helper::bit_cast<f_to_uint_t<T>>(valf);

    if constexpr (std::is_same_v<T, float>)
    {
        snan_raw = 0x7f800001U;
    }
    else
    {
        snan_raw = 0x7ff0000000000001ULL;
    }

    return snan_raw == val;
}

template <typename T> static T fdround(T val, Cpu& cpu, Decoder decoder)
{
    T newval = val;

    switch (decoder.fp_rounding_mode())
    {
    case FPURoundigMode::RoundToNearest:
    case FPURoundigMode::RoundNearestMaxMagnitude:
        newval = std::round(val);
        break;
    case FPURoundigMode::RoundToZero:
        newval = std::trunc(val);
        break;
    case FPURoundigMode::RoundDown:
        newval = std::floor(val);
        break;
    case FPURoundigMode::RoundUp:
        newval = std::ceil(val);
        break;
    case FPURoundigMode::RoundDynamic:
        break;
    default:
        [[unlikely]] cpu.set_exception(exception::Exception::IllegalInstruction);
        break;
    }

    if (newval != val) [[unlikely]]
    {
        cpu.cregs.set_fpu_exception(csr::FExcept::Inexact);
    }

    return newval;
}

template <typename T, typename T1> T fd_conv_check(T1 val, Cpu& cpu)
{
    constexpr T min = std::numeric_limits<T>::min();
    constexpr T max = std::numeric_limits<T>::max();

    bool val_is_nan = std::isnan(val);
    T ret = val;

    if (val_is_nan || val < min || val >= max) [[unlikely]]
    {
        cpu.cregs.set_fpu_exception(csr::FExcept::Invalid);
    }

    if (val_is_nan)
    {
        val = max;
    }

    return val;
}

template <typename T> static void fs(Cpu& cpu, Decoder decoder)
{
    Cpu::reg_name rs1 = decoder.rs1();
    Cpu::reg_name rs2 = decoder.rs2();
    uint64_t offset = decoder.fs_offset();

    uint64_t address = cpu.regs[rs1] + offset;

    uint64_t rs2f_bits = cpu.fregs[rs2];

    cpu.mmu.store(address, rs2f_bits, sizeof(T) * 8);
}

template <typename T> static void fl(Cpu& cpu, Decoder decoder)
{
    Cpu::reg_name rs1 = decoder.rs1();
    Cpu::reg_name rd = decoder.rd();
    uint64_t offset = decoder.fl_offset();

    uint64_t address = cpu.regs[rs1] + offset;

    uint64_t value = cpu.mmu.load(address, sizeof(T) * 8);

    if constexpr (std::is_same_v<T, float>)
    {
        value |= 0xffffffff00000000ULL;
    }

    cpu.fregs[rd] = value;
}

template <typename T> static void fmadd(Cpu& cpu, Decoder decoder)
{
    Cpu::reg_name rs1 = decoder.rs1();
    Cpu::reg_name rs2 = decoder.rs2();
    Cpu::reg_name rs3 = decoder.rs3();
    Cpu::reg_name rd = decoder.rd();

    T val1 = cpu.fregs[rs1];
    T val2 = cpu.fregs[rs2];
    T val3 = cpu.fregs[rs3];

    T result = val1 * val2 + val3;

    if (is_cnan(result)) [[unlikely]]
    {
        cpu.cregs.set_fpu_exception(csr::FExcept::Invalid);
    }

    cpu.fregs[rd] = result;
}

template <typename T> static void fmsub(Cpu& cpu, Decoder decoder)
{
    Cpu::reg_name rs1 = decoder.rs1();
    Cpu::reg_name rs2 = decoder.rs2();
    Cpu::reg_name rs3 = decoder.rs3();
    Cpu::reg_name rd = decoder.rd();

    T val1 = cpu.fregs[rs1];
    T val2 = cpu.fregs[rs2];
    T val3 = cpu.fregs[rs3];

    T result = val1 * val2 - val3;

    if (is_cnan(result)) [[unlikely]]
    {
        cpu.cregs.set_fpu_exception(csr::FExcept::Invalid);
    }

    cpu.fregs[rd] = result;
}

template <typename T> static void fnmadd(Cpu& cpu, Decoder decoder)
{
    Cpu::reg_name rs1 = decoder.rs1();
    Cpu::reg_name rs2 = decoder.rs2();
    Cpu::reg_name rs3 = decoder.rs3();
    Cpu::reg_name rd = decoder.rd();

    T val1 = -cpu.fregs[rs1].get_value<T>();
    T val2 = cpu.fregs[rs2];
    T val3 = cpu.fregs[rs3];

    T result = val1 * val2 + val3;

    if (is_cnan(result)) [[unlikely]]
    {
        cpu.cregs.set_fpu_exception(csr::FExcept::Invalid);
    }

    cpu.fregs[rd] = result;
}

template <typename T> static void fnmsub(Cpu& cpu, Decoder decoder)
{
    Cpu::reg_name rs1 = decoder.rs1();
    Cpu::reg_name rs2 = decoder.rs2();
    Cpu::reg_name rs3 = decoder.rs3();
    Cpu::reg_name rd = decoder.rd();

    T val1 = -cpu.fregs[rs1].get_value<T>();
    T val2 = cpu.fregs[rs2];
    T val3 = cpu.fregs[rs3];

    T result = val1 * val2 - val3;

    if (is_cnan(result)) [[unlikely]]
    {
        cpu.cregs.set_fpu_exception(csr::FExcept::Invalid);
    }

    cpu.fregs[rd] = result;
}

template <typename T> static void fadd(Cpu& cpu, Decoder decoder)
{
    Cpu::reg_name rs1 = decoder.rs1();
    Cpu::reg_name rs2 = decoder.rs2();
    Cpu::reg_name rd = decoder.rd();

    T val1 = cpu.fregs[rs1];
    T val2 = cpu.fregs[rs2];

    T result = val1 + val2;

    if (is_cnan(result)) [[unlikely]]
    {
        cpu.cregs.set_fpu_exception(csr::FExcept::Invalid);
    }

    cpu.fregs[rd] = result;
}

template <typename T> static void fsub(Cpu& cpu, Decoder decoder)
{
    Cpu::reg_name rs1 = decoder.rs1();
    Cpu::reg_name rs2 = decoder.rs2();
    Cpu::reg_name rd = decoder.rd();

    T val1 = cpu.fregs[rs1];
    T val2 = cpu.fregs[rs2];

    T result = val1 - val2;

    if (is_cnan(result)) [[unlikely]]
    {
        cpu.cregs.set_fpu_exception(csr::FExcept::Invalid);
    }

    cpu.fregs[rd] = result;
}

template <typename T> static void fmul(Cpu& cpu, Decoder decoder)
{
    Cpu::reg_name rs1 = decoder.rs1();
    Cpu::reg_name rs2 = decoder.rs2();
    Cpu::reg_name rd = decoder.rd();

    T val1 = cpu.fregs[rs1];
    T val2 = cpu.fregs[rs2];

    T result = val1 * val2;

    if (is_cnan(result)) [[unlikely]]
    {
        cpu.cregs.set_fpu_exception(csr::FExcept::Invalid);
    }

    cpu.fregs[rd] = result;
}

template <typename T> static void fdiv(Cpu& cpu, Decoder decoder)
{
    Cpu::reg_name rs1 = decoder.rs1();
    Cpu::reg_name rs2 = decoder.rs2();
    Cpu::reg_name rd = decoder.rd();

    T val1 = cpu.fregs[rs1];
    T val2 = cpu.fregs[rs2];

    T result = val1 / val2;

    if (is_cnan(result)) [[unlikely]]
    {
        cpu.cregs.set_fpu_exception(csr::FExcept::Invalid);
    }

    cpu.fregs[rd] = result;
}

template <typename T> static void fmin(Cpu& cpu, Decoder decoder)
{
    Cpu::reg_name rs1 = decoder.rs1();
    Cpu::reg_name rs2 = decoder.rs2();
    Cpu::reg_name rd = decoder.rd();

    T val1 = cpu.fregs[rs1];
    T val2 = cpu.fregs[rs2];

    bool isval1nan = std::isnan(val1);
    bool isval2nan = std::isnan(val2);

    if (is_snan(val1) || is_snan(val2)) [[unlikely]]
    {
        cpu.cregs.set_fpu_exception(csr::FExcept::Invalid);
    }

    if (isval1nan && isval2nan) [[unlikely]]
    {
        cpu.fregs[rd] = get_canonical_nan<T>();
    }
    else if (isval1nan) [[unlikely]]
    {
        cpu.fregs[rd] = val2;
    }
    else if (isval2nan) [[unlikely]]
    {
        cpu.fregs[rd] = val1;
    }
    else if (val1 < val2)
    {
        cpu.fregs[rd] = val1;
    }
    else if (val1 > val2)
    {
        cpu.fregs[rd] = val2;
    }
    else [[unlikely]]
    {
        cpu.fregs[rd] = signbit(val1) ? val1 : val2;
    }
}

template <typename T> static void fmax(Cpu& cpu, Decoder decoder)
{
    Cpu::reg_name rs1 = decoder.rs1();
    Cpu::reg_name rs2 = decoder.rs2();
    Cpu::reg_name rd = decoder.rd();

    T val1 = cpu.fregs[rs1];
    T val2 = cpu.fregs[rs2];

    bool isval1nan = std::isnan(val1);
    bool isval2nan = std::isnan(val2);

    if (is_snan(val1) || is_snan(val2)) [[unlikely]]
    {
        cpu.cregs.set_fpu_exception(csr::FExcept::Invalid);
    }

    if (isval1nan && isval2nan) [[unlikely]]
    {
        cpu.fregs[rd] = get_canonical_nan<T>();
    }
    else if (isval1nan) [[unlikely]]
    {
        cpu.fregs[rd] = val2;
    }
    else if (isval2nan) [[unlikely]]
    {
        cpu.fregs[rd] = val1;
    }
    else if (val1 > val2)
    {
        cpu.fregs[rd] = val1;
    }
    else if (val1 < val2)
    {
        cpu.fregs[rd] = val2;
    }
    else [[unlikely]]
    {
        cpu.fregs[rd] = signbit(val1) ? val2 : val1;
    }
}

template <typename T, typename T1> static void fcvt(Cpu& cpu, Decoder decoder)
{
    Cpu::reg_name rs1 = decoder.rs1();
    Cpu::reg_name rd = decoder.rd();

    T val = static_cast<T>(cpu.fregs[rs1].get_value<T1>());

    if (std::isnan(val)) [[unlikely]]
    {
        val = std::numeric_limits<T>::quiet_NaN();
    }

    cpu.fregs[rd] = val;
}

template <typename T> static void fsqrt(Cpu& cpu, Decoder decoder)
{
    Cpu::reg_name rs1 = decoder.rs1();
    Cpu::reg_name rs2 = decoder.rs2();
    Cpu::reg_name rd = decoder.rd();

    T val1 = cpu.fregs[rs1];

    T result = std::sqrt(val1);

    if (val1 < static_cast<T>(0)) [[unlikely]]
    {
        cpu.cregs.set_fpu_exception(csr::FExcept::Invalid);
    }

    cpu.fregs[rd] = result;
}

template <typename T> static void fle(Cpu& cpu, Decoder decoder)
{
    Cpu::reg_name rs1 = decoder.rs1();
    Cpu::reg_name rs2 = decoder.rs2();
    Cpu::reg_name rd = decoder.rd();

    T val1 = cpu.fregs[rs1];
    T val2 = cpu.fregs[rs2];

    bool result = val1 <= val2;

    if (std::isnan(val1) || std::isnan(val2)) [[unlikely]]
    {
        cpu.cregs.set_fpu_exception(csr::FExcept::Invalid);
    }

    cpu.regs[rd] = result;
}

template <typename T> static void flt(Cpu& cpu, Decoder decoder)
{
    Cpu::reg_name rs1 = decoder.rs1();
    Cpu::reg_name rs2 = decoder.rs2();
    Cpu::reg_name rd = decoder.rd();

    T val1 = cpu.fregs[rs1];
    T val2 = cpu.fregs[rs2];

    bool result = val1 < val2;

    if (std::isnan(val1) || std::isnan(val2)) [[unlikely]]
    {
        cpu.cregs.set_fpu_exception(csr::FExcept::Invalid);
    }

    cpu.regs[rd] = result;
}

template <typename T> static void feq(Cpu& cpu, Decoder decoder)
{
    Cpu::reg_name rs1 = decoder.rs1();
    Cpu::reg_name rs2 = decoder.rs2();
    Cpu::reg_name rd = decoder.rd();

    T val1 = cpu.fregs[rs1];
    T val2 = cpu.fregs[rs2];

    T result = val1 == val2;

    if (is_snan(val1) || is_snan(val2)) [[unlikely]]
    {
        cpu.cregs.set_fpu_exception(csr::FExcept::Invalid);
    }

    cpu.regs[rd] = result;
}

template <typename T> static void fsgnj(Cpu& cpu, Decoder decoder)
{
    Cpu::reg_name rs1 = decoder.rs1();
    Cpu::reg_name rs2 = decoder.rs2();
    Cpu::reg_name rd = decoder.rd();

    T val1 = cpu.fregs[rs1];
    T val2 = cpu.fregs[rs2];

    T result = std::copysign(val1, val2);

    cpu.fregs[rd] = result;
}

template <typename T> static void fsgnjn(Cpu& cpu, Decoder decoder)
{
    Cpu::reg_name rs1 = decoder.rs1();
    Cpu::reg_name rs2 = decoder.rs2();
    Cpu::reg_name rd = decoder.rd();

    T val1 = cpu.fregs[rs1];
    T val2 = -cpu.fregs[rs2].get_value<T>();

    T result = std::copysign(val1, val2);

    cpu.fregs[rd] = result;
}

template <typename T> static void fsgnjx(Cpu& cpu, Decoder decoder)
{
    Cpu::reg_name rs1 = decoder.rs1();
    Cpu::reg_name rs2 = decoder.rs2();
    Cpu::reg_name rd = decoder.rd();

    f_to_uint_t<T> intval1 = cpu.fregs[rs1];
    f_to_uint_t<T> intval2 = cpu.fregs[rs2];

    f_to_uint_t<T> mask = 1ULL << (sizeof(intval1) * 8 - 1);

    f_to_uint_t<T> result = intval1 & (mask - 1);
    result |= (intval1 & mask) ^ (intval2 & mask);

    cpu.fregs[rd] = result;
}

template <typename T> static void fcvtsd(Cpu& cpu, Decoder decoder)
{
    Cpu::reg_name rs1 = decoder.rs1();
    Cpu::reg_name rs2 = decoder.rs2();
    Cpu::reg_name rd = decoder.rd();

    T val = fdround<T>(cpu.fregs[rs1], cpu, decoder);

    switch (static_cast<uint64_t>(decoder.rs2()))
    {
    case FDType::FCVT0:
        cpu.regs[rd] = SIGNEXTEND_CAST(fd_conv_check<int32_t>(val, cpu), int32_t);
        break;
    case FDType::FCVT1:
        cpu.regs[rd] = SIGNEXTEND_CAST(fd_conv_check<uint32_t>(val, cpu), int32_t);
        break;
    case FDType::FCVT2:
        cpu.regs[rd] = fd_conv_check<int64_t>(val, cpu);
        break;
    case FDType::FCVT3:
        cpu.regs[rd] = fd_conv_check<uint64_t>(val, cpu);
        break;
    default:
        [[unlikely]] cpu.set_exception(exception::Exception::IllegalInstruction, decoder.insn);
        break;
    }
}

template <typename T> static void fcvtsdw(Cpu& cpu, Decoder decoder)
{
    Cpu::reg_name rs1 = decoder.rs1();
    Cpu::reg_name rs2 = decoder.rs2();
    Cpu::reg_name rd = decoder.rd();

    int64_t val = cpu.regs[rs1];

    switch (static_cast<uint64_t>(decoder.rs2()))
    {
    case FDType::FCVT0:
        cpu.fregs[rd] = static_cast<T>(static_cast<int32_t>(val));
        break;
    case FDType::FCVT1:
        cpu.fregs[rd] = static_cast<T>(static_cast<uint32_t>(val));
        break;
    case FDType::FCVT2:
        cpu.fregs[rd] = static_cast<T>(val);
        break;
    case FDType::FCVT3:
        cpu.fregs[rd] = static_cast<T>(static_cast<uint64_t>(val));
        break;
    default:
        [[unlikely]] cpu.set_exception(exception::Exception::IllegalInstruction, decoder.insn);
        break;
    }
}

template <typename T> static void fmvxwd(Cpu& cpu, Decoder decoder)
{
    Cpu::reg_name rs1 = decoder.rs1();
    Cpu::reg_name rd = decoder.rd();

    f_to_uint_t<T> val = cpu.fregs[rs1];

    cpu.regs[rd] = SIGNEXTEND_CAST(val, std::make_signed_t<f_to_uint_t<T>>);
}

template <typename T> static void fclass(Cpu& cpu, Decoder decoder)
{
    Cpu::reg_name rs1 = decoder.rs1();
    Cpu::reg_name rd = decoder.rd();

    T val = cpu.fregs[rs1];
    bool neg_val = std::signbit(val);

    switch (std::fpclassify(val))
    {
    case FP_INFINITE:
        cpu.regs[rd] = neg_val ? fdtype::FClass::INF : fdtype::FClass::POS_INF;
        break;
    case FP_NORMAL:
        cpu.regs[rd] = neg_val ? fdtype::FClass::NORMAL : fdtype::FClass::POS_NORMAL;
        break;
    case FP_SUBNORMAL:
        cpu.regs[rd] = neg_val ? fdtype::FClass::SUBNORMAL : fdtype::FClass::POS_SUBNORMAL;
        break;
    case FP_ZERO:
        cpu.regs[rd] = neg_val ? fdtype::FClass::NEG_ZERO : fdtype::FClass::POS_ZERO;
        break;
    case FP_NAN:
        cpu.regs[rd] = is_snan(val) ? fdtype::FClass::NAN_SIG : fdtype::FClass::NAN_QUIET;
        break;
    }
}
template <typename T> static void fmvx(Cpu& cpu, Decoder decoder)
{
    Cpu::reg_name rs1 = decoder.rs1();
    Cpu::reg_name rd = decoder.rd();

    cpu.fregs[rd] = cpu.regs[rs1];
}

static void set_exceptions(Cpu& cpu)
{
    if (std::fetestexcept(FE_DIVBYZERO))
    {
        cpu.cregs.set_fpu_exception(csr::FExcept::DivByZero);
    }

    if (std::fetestexcept(FE_INEXACT))
    {
        cpu.cregs.set_fpu_exception(csr::FExcept::Inexact);
    }

    // if (std::fetestexcept(FE_INVALID))
    // {
    //     cpu.cregs.set_fpu_exception(csr::FExcept::Invalid);
    // }

    if (std::fetestexcept(FE_OVERFLOW))
    {
        cpu.cregs.set_fpu_exception(csr::FExcept::Overflow);
    }

    if (std::fetestexcept(FE_UNDERFLOW))
    {
        cpu.cregs.set_fpu_exception(csr::FExcept::Undeflow);
    }

    if (std::fetestexcept(FE_ALL_EXCEPT))
    {
        std::feclearexcept(FE_ALL_EXCEPT);
    }
}

bool check_fs(Cpu& cpu)
{
    if (!cpu.cregs.is_fpu_enabled())
    {
        // cpu.set_exception(exception::Exception::IllegalInstruction);
        return false;
    }

    return true;
}

} // namespace fdimpl

void fdtype::fs(Cpu& cpu, Decoder decoder)
{
    if (!fdimpl::check_fs(cpu))
    {
        return;
    }

    switch (decoder.funct3())
    {
    case FDType::FSW:
        fdimpl::fs<float>(cpu, decoder);
        break;
    case FDType::FSD:
        fdimpl::fs<double>(cpu, decoder);
        break;
    default:
        [[unlikely]] cpu.set_exception(exception::Exception::IllegalInstruction, decoder.insn);
        break;
    }

    fdimpl::set_exceptions(cpu);
}

void fdtype::fl(Cpu& cpu, Decoder decoder)
{
    if (!fdimpl::check_fs(cpu))
    {
        return;
    }

    switch (decoder.funct3())
    {
    case FDType::FLW:
        fdimpl::fl<float>(cpu, decoder);
        break;
    case FDType::FLD:
        fdimpl::fl<double>(cpu, decoder);
        break;
    default:
        [[unlikely]] cpu.set_exception(exception::Exception::IllegalInstruction, decoder.insn);
        break;
    }

    fdimpl::set_exceptions(cpu);
}

void fdtype::fmadd(Cpu& cpu, Decoder decoder)
{
    if (!fdimpl::check_fs(cpu))
    {
        return;
    }

    switch (decoder.funct2())
    {
    case FDType::FMADDS:
        fdimpl::fmadd<float>(cpu, decoder);
        break;
    case FDType::FMADDD:
        fdimpl::fmadd<double>(cpu, decoder);
        break;
    default:
        [[unlikely]] cpu.set_exception(exception::Exception::IllegalInstruction, decoder.insn);
        break;
    }

    fdimpl::set_exceptions(cpu);
}

void fdtype::fmsub(Cpu& cpu, Decoder decoder)
{
    if (!fdimpl::check_fs(cpu))
    {
        return;
    }

    switch (decoder.funct2())
    {
    case FDType::FMSUBS:
        fdimpl::fmsub<float>(cpu, decoder);
        break;
    case FDType::FMSUBD:
        fdimpl::fmsub<double>(cpu, decoder);
        break;
    default:
        [[unlikely]] cpu.set_exception(exception::Exception::IllegalInstruction, decoder.insn);
        break;
    }

    fdimpl::set_exceptions(cpu);
}

void fdtype::fnmadd(Cpu& cpu, Decoder decoder)
{
    if (!fdimpl::check_fs(cpu))
    {
        return;
    }

    switch (decoder.funct2())
    {
    case FDType::FNMADDS:
        fdimpl::fnmadd<float>(cpu, decoder);
        break;
    case FDType::FNMADDD:
        fdimpl::fnmadd<double>(cpu, decoder);
        break;
    default:
        [[unlikely]] cpu.set_exception(exception::Exception::IllegalInstruction, decoder.insn);
        break;
    }

    fdimpl::set_exceptions(cpu);
}

void fdtype::fnmsub(Cpu& cpu, Decoder decoder)
{
    if (!fdimpl::check_fs(cpu))
    {
        return;
    }

    switch (decoder.funct2())
    {
    case FDType::FNMSUBS:
        fdimpl::fnmsub<float>(cpu, decoder);
        break;
    case FDType::FNMSUBD:
        fdimpl::fnmsub<double>(cpu, decoder);
        break;
    default:
        [[unlikely]] cpu.set_exception(exception::Exception::IllegalInstruction, decoder.insn);
        break;
    }

    fdimpl::set_exceptions(cpu);
}

void fdtype::fother(Cpu& cpu, Decoder decoder)
{
    if (!fdimpl::check_fs(cpu))
    {
        return;
    }

    switch (decoder.funct7())
    {
    case FDType::FADDS:
        fdimpl::fadd<float>(cpu, decoder);
        break;
    case FDType::FADDD:
        fdimpl::fadd<double>(cpu, decoder);
        break;
    case FDType::FSUBS:
        fdimpl::fsub<float>(cpu, decoder);
        break;
    case FDType::FSUBD:
        fdimpl::fsub<double>(cpu, decoder);
        break;
    case FDType::FMULS:
        fdimpl::fmul<float>(cpu, decoder);
        break;
    case FDType::FMULD:
        fdimpl::fmul<double>(cpu, decoder);
        break;
    case FDType::FDIVS:
        fdimpl::fdiv<float>(cpu, decoder);
        break;
    case FDType::FDIVD:
        fdimpl::fdiv<double>(cpu, decoder);
        break;
    case FDType::FSNGJS:
        switch (decoder.funct3())
        {
        case FDType::FSGNJ:
            fdimpl::fsgnj<float>(cpu, decoder);
            break;
        case FDType::FSGNJN:
            fdimpl::fsgnjn<float>(cpu, decoder);
            break;
        case FDType::FSGNJX:
            fdimpl::fsgnjx<float>(cpu, decoder);
            break;
        default:
            [[unlikely]] cpu.set_exception(exception::Exception::IllegalInstruction, decoder.insn);
            break;
        }
        break;
    case FDType::FSNGJD:
        switch (decoder.funct3())
        {
        case FDType::FSGNJ:
            fdimpl::fsgnj<double>(cpu, decoder);
            break;
        case FDType::FSGNJN:
            fdimpl::fsgnjn<double>(cpu, decoder);
            break;
        case FDType::FSGNJX:
            fdimpl::fsgnjx<double>(cpu, decoder);
            break;
        default:
            [[unlikely]] cpu.set_exception(exception::Exception::IllegalInstruction, decoder.insn);
            break;
        }
        break;
    case FDType::FMINMAXS:
        switch (decoder.funct3())
        {
        case FDType::MIN:
            fdimpl::fmin<float>(cpu, decoder);
            break;
        case FDType::MAX:
            fdimpl::fmax<float>(cpu, decoder);
            break;
        default:
            [[unlikely]] cpu.set_exception(exception::Exception::IllegalInstruction, decoder.insn);
            break;
        }
        break;
    case FDType::FMINMAXD:
        switch (decoder.funct3())
        {
        case FDType::MIN:
            fdimpl::fmin<double>(cpu, decoder);
            break;
        case FDType::MAX:
            fdimpl::fmax<double>(cpu, decoder);
            break;
        default:
            [[unlikely]] cpu.set_exception(exception::Exception::IllegalInstruction, decoder.insn);
            break;
        }
        break;
    case FDType::FCVTSD:
        fdimpl::fcvt<float, double>(cpu, decoder);
        break;
    case FDType::FCVTDS:
        fdimpl::fcvt<double, float>(cpu, decoder);
        break;
    case FDType::FSQRTS:
        fdimpl::fsqrt<float>(cpu, decoder);
        break;
    case FDType::FSQRTD:
        fdimpl::fsqrt<double>(cpu, decoder);
        break;
    case FDType::FCS:
        switch (decoder.funct3())
        {
        case FDType::FLE:
            fdimpl::fle<float>(cpu, decoder);
            break;
        case FDType::FLT:
            fdimpl::flt<float>(cpu, decoder);
            break;
        case FDType::FEQ:
            fdimpl::feq<float>(cpu, decoder);
            break;
        default:
            [[unlikely]] cpu.set_exception(exception::Exception::IllegalInstruction, decoder.insn);
            break;
        }
        break;
    case FDType::FCD:
        switch (decoder.funct3())
        {
        case FDType::FLE:
            fdimpl::fle<double>(cpu, decoder);
            break;
        case FDType::FLT:
            fdimpl::flt<double>(cpu, decoder);
            break;
        case FDType::FEQ:
            fdimpl::feq<double>(cpu, decoder);
            break;
        default:
            [[unlikely]] cpu.set_exception(exception::Exception::IllegalInstruction, decoder.insn);
            break;
        }
        break;
    case FDType::FCVTS:
        fdimpl::fcvtsd<float>(cpu, decoder);
        break;
    case FDType::FCVTD:
        fdimpl::fcvtsd<double>(cpu, decoder);
        break;
    case FDType::FCVTSW:
        fdimpl::fcvtsdw<float>(cpu, decoder);
        break;
    case FDType::FCVTDW:
        fdimpl::fcvtsdw<double>(cpu, decoder);
        break;
    case FDType::FMVXW:
        switch (decoder.funct3())
        {
        case FDType::FMV:
            fdimpl::fmvxwd<float>(cpu, decoder);
            break;
        case FDType::FCLASS:
            fdimpl::fclass<float>(cpu, decoder);
            break;
        default:
            [[unlikely]] cpu.set_exception(exception::Exception::IllegalInstruction, decoder.insn);
            break;
        }
        break;
    case FDType::FMVXD:
        switch (decoder.funct3())
        {
        case FDType::FMV:
            fdimpl::fmvxwd<double>(cpu, decoder);
            break;
        case FDType::FCLASS:
            fdimpl::fclass<double>(cpu, decoder);
            break;
        default:
            [[unlikely]] cpu.set_exception(exception::Exception::IllegalInstruction, decoder.insn);
            break;
        }
        break;
    case FDType::FMVWX:
        fdimpl::fmvx<float>(cpu, decoder);
        break;
    case FDType::FMVDX:
        fdimpl::fmvx<double>(cpu, decoder);
        break;
    default:
        [[unlikely]] cpu.set_exception(exception::Exception::IllegalInstruction, decoder.insn);
        break;
    }

    fdimpl::set_exceptions(cpu);
}
