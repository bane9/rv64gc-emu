#pragma once

#include "common_def.hpp"
#include "cpu.hpp"
#include <cstdint>
#include <ostream>

enum class OpcodeType : uint64_t
{
    COMPRESSED_QUANDRANT0 = 0x00,
    COMPRESSED_QUANDRANT1 = 0x01,
    COMPRESSED_QUANDRANT2 = 0x02,

    LOAD = 0x03,

    FENCE = 0x0f,

    AUIPC = 0x17,
    LUI = 0x37,

    JALR = 0x67,
    JAL = 0x6f,

    I = 0x13,
    S = 0x23,
    R = 0x33,
    B = 0x63,

    FL = 0x07,
    FS = 0x27,
    FMADD = 0x43,
    FMSUB = 0x47,
    FNMADD = 0x4b,
    FNMSUB = 0x4f,
    FOTHER = 0x53,

    ATOMIC = 0x2f,

    I64 = 0x1b,
    R64 = 0x3b,

    CSR = 0x73,
};

struct LOADType
{
    enum funct3 : uint64_t
    {
        LB = 0x00,
        LH = 0x01,
        LW = 0x02,
        LD = 0x03,
        LBU = 0x04,
        LHU = 0x05,
        LWU = 0x06
    };
};

struct IType
{
    enum funct3 : uint64_t
    {
        ADDI = 0x00,
        SLLI = 0x01,
        SLTI = 0x02,
        SLTIU = 0x03,
        XORI = 0x04,
        SRI = 0x05,
        ORI = 0x06,
        ANDI = 0x07
    };

    enum SRIfunct7 : uint64_t
    {
        SRLI = 0x00,
        SRAI = 0x10
    };
};

struct SType
{
    enum funct3 : uint64_t
    {
        SB = 0x00,
        SH = 0x01,
        SW = 0x02,
        SD = 0x03
    };
};

struct RType
{
    enum funct3 : uint64_t
    {
        ADDMULSUB = 0x00,
        SLLMULH = 0x01,
        SLTMULHSU = 0x02,
        SLTUMULHU = 0x03,
        XORDIV = 0x04,
        SR = 0x05,
        ORREM = 0x06,
        ANDREMU = 0x07
    };

    enum ADDSUBfunct7 : uint64_t
    {
        ADD = 0x00,
        MUL = 0x01,
        SUB = 0x20
    };

    enum SLLMULHfunct7 : uint64_t
    {
        SLL = 0x00,
        MULH = 0x01,
    };

    enum SLTMULHSUfunct7 : uint64_t
    {
        SLT = 0x00,
        MULHSU = 0x01,
    };

    enum SLTUMULHUfunct7 : uint64_t
    {
        SLTU = 0x00,
        MULHU = 0x01,
    };

    enum XORDIVfunct7 : uint64_t
    {
        XOR = 0x00,
        DIV = 0x01,
    };

    enum SRfunct7 : uint64_t
    {
        SRL = 0x00,
        DIVU = 0x01,
        SRA = 0x20
    };

    enum ORREMfunct7 : uint64_t
    {
        OR = 0x00,
        REM = 0x01,
    };
    enum ANDREMUfunct7 : uint64_t
    {
        AND = 0x00,
        REMU = 0x01,
    };
};

struct AtomicType
{
    enum funct3 : uint64_t
    {
        AMOW = 0x02,
        AMOD = 0x03
    };

    enum funct5 : uint64_t
    {
        ADD = 0x00,
        SWAP = 0x01,
        LR = 0x02,
        SC = 0x03,
        XOR = 0x04,
        OR = 0x08,
        AND = 0x0c,
        MIN = 0x10,
        MAX = 0x14,
        MINU = 0x18,
        MAXU = 0x1c,
    };
};

struct BType
{
    enum funct3 : uint64_t
    {
        BEQ = 0x00,
        BNE = 0x01,
        BLT = 0x04,
        BGE = 0x05,
        BLTU = 0x06,
        BGEU = 0x07
    };
};

struct I64Type
{
    enum funct3 : uint64_t
    {
        ADDIW = 0x00,
        SLLIW = 0x01,
        SRIW = 0x05
    };

    enum SRIWfunt7 : uint64_t
    {
        SRLIW = 0x00,
        SRAIW = 0x20
    };
};

struct R64Type
{
    enum funct3 : uint64_t
    {
        ADDSUBW = 0x00,
        SLLW = 0x01,
        DIVW = 0x04,
        SRW = 0x05,
        REMW = 0x06,
        REMUW = 0x07
    };

    enum ADDSUBWfunct7 : uint64_t
    {
        ADDW = 0x00,
        MULW = 0x01,
        SUBW = 0x20
    };

    enum SRWfunct7 : uint64_t
    {
        SRLW = 0x00,
        DIVUW = 0x01,
        SRAW = 0x20
    };
};

struct CsrType
{
    enum funct3 : uint64_t
    {
        ENVIRONMENT = 0x00,
        CSRW = 0x01,
        CSRS = 0x02,
        CSRC = 0x03,
        CSRWI = 0x05,
        CSRSI = 0x06,
        CSRCI = 0x07,
    };

    enum EnvironmentImm : uint64_t
    {
        ECALL = 0x00,
        EBREAK = 0x01,
        RET = 0x02,
        WFI = 0x05,
        OTHER
    };

    enum Environmentfunct7 : uint64_t
    {
        ECALL7 = 0x00,
        EBREAK7 = 0x00,
        URET7 = 0x00,
        SRET7 = 0x08,
        MRET7 = 0x18,
        WFI7 = 0x08,
        SFENCEVMA7 = 0x09,
        HFENCEBVMA7 = 0x11,
        HFENCEGVMA7 = 0x51
    };
};

struct FDType
{
    enum FLfunct3 : uint64_t
    {
        FLW = 0x02,
        FLD = 0x03
    };

    enum FSfunct3 : uint64_t
    {
        FSW = 0x02,
        FSD = 0x03
    };

    enum FMADDfunct2 : uint64_t
    {
        FMADDS = 0x00,
        FMADDD = 0x01
    };

    enum FMSUBfunct2 : uint64_t
    {
        FMSUBS = 0x00,
        FMSUBD = 0x01
    };

    enum FNMADDfunct2 : uint64_t
    {
        FNMADDS = 0x00,
        FNMADDD = 0x01
    };

    enum FNMSUBfunct2 : uint64_t
    {
        FNMSUBS = 0x00,
        FNMSUBD = 0x01
    };

    enum FOTHERfunct7 : uint64_t
    {
        FADDS = 0x00,
        FADDD = 0x01,
        FSUBS = 0x04,
        FSUBD = 0x05,
        FMULS = 0x08,
        FMULD = 0x09,
        FDIVS = 0x0c,
        FDIVD = 0x0d,

        FSNGJS = 0x10, // f3
        FSNGJD = 0x11, // f3

        FMINMAXS = 0x14, // f3
        FMINMAXD = 0x15, // f3

        FCVTSD = 0x20,
        FCVTDS = 0x21,
        FSQRTS = 0x2c,
        FSQRTD = 0x2d,

        FCS = 0x50, // f3
        FCD = 0x51, // f3

        FCVTS = 0x60, // rs2
        FCVTD = 0x61, // rs2

        FCVTSW = 0x68, // rs2
        FCVTDW = 0x69, // rs2

        FMVXW = 0x70, // f3
        FMVXD = 0x71, // f3

        FMVWX = 0x78,
        FMVDX = 0x79,
    };

    enum FSGNJfunct3 : uint64_t
    {
        FSGNJ = 0x00,
        FSGNJN = 0x01,
        FSGNJX = 0x02
    };

    enum FMINMAXfunct3 : uint64_t
    {
        MIN = 0x00,
        MAX = 0x01
    };

    enum FCfunct3 : uint64_t
    {
        FLE = 0x00,
        FLT = 0x01,
        FEQ = 0x02
    };

    enum FCVTrs2 : uint64_t
    {
        FCVT0 = 0x00,
        FCVT1 = 0x01,
        FCVT2 = 0x02,
        FCVT3 = 0x03
    };

    enum FMVfunct3 : uint64_t
    {
        FMV = 0x00,
        FCLASS = 0x01
    };
};

class Decoder
{
  public:
    Decoder(uint32_t insn);

  public:
    uint64_t opcode() const;
    OpcodeType opcode_type() const;
    Cpu::reg_name rd() const;
    Cpu::reg_name rs1() const;
    Cpu::reg_name rs2() const;
    Cpu::reg_name rs3() const;
    uint64_t imm_i() const;
    uint64_t imm_s() const;
    uint64_t imm_b() const;
    uint64_t imm_u() const;
    uint64_t imm_j() const;
    uint64_t funct2() const;
    uint64_t funct3() const;
    uint64_t funct5() const;
    uint64_t funct7() const;
    uint64_t fs_offset() const;
    uint64_t fl_offset() const;
    uint32_t shamt() const;
    uint64_t csr() const;

    uint64_t insn_size() const;

    uint64_t compressed_rd() const;
    uint64_t compressed_rs1() const;
    uint64_t compressed_rs2() const;
    uint64_t compressed_shamt() const;

    uint64_t compressed_opcode() const;
    uint64_t compressed_funct3() const;
    uint64_t compressed_funct2() const;

    FPURoundigMode::Mode fp_rounding_mode() const;

  public:
    void dump(std::ostream& stream) const;

  public:
    const uint32_t insn;
};
