#include "atomictype.hpp"
#include "helper.hpp"
#include <utility>

static inline void amow(Cpu& cpu, Decoder decoder)
{
    if (cpu.regs[decoder.rs1()] % 4 != 0)
    {
        cpu.set_exception(exception::Exception::InstructionAddressMisaligned, decoder.insn);
    }

    switch (decoder.funct5())
    {
    case AtomicType::ADD:
        atomic::amoaddw(cpu, decoder);
        break;
    case AtomicType::SWAP:
        atomic::amoswapw(cpu, decoder);
        break;
    case AtomicType::LR:
        atomic::lrw(cpu, decoder);
        break;
    case AtomicType::SC:
        atomic::scw(cpu, decoder);
        break;
    case AtomicType::XOR:
        atomic::amoxorw(cpu, decoder);
        break;
    case AtomicType::OR:
        atomic::amoorw(cpu, decoder);
        break;
    case AtomicType::AND:
        atomic::amoandw(cpu, decoder);
        break;
    case AtomicType::MIN:
        atomic::amominw(cpu, decoder);
        break;
    case AtomicType::MAX:
        atomic::amomaxw(cpu, decoder);
        break;
    case AtomicType::MINU:
        atomic::amominuw(cpu, decoder);
        break;
    case AtomicType::MAXU:
        atomic::amomaxuw(cpu, decoder);
        break;
    default:
        cpu.set_exception(exception::Exception::IllegalInstruction, decoder.insn);
        break;
    }
}

static inline void amod(Cpu& cpu, Decoder decoder)
{
    if (cpu.regs[decoder.rs1()] % 8 != 0)
    {
        cpu.set_exception(exception::Exception::IllegalInstruction, decoder.insn);
    }

    switch (decoder.funct5())
    {
    case AtomicType::ADD:
        atomic::amoaddd(cpu, decoder);
        break;
    case AtomicType::SWAP:
        atomic::amoswapd(cpu, decoder);
        break;
    case AtomicType::LR:
        atomic::lrd(cpu, decoder);
        break;
    case AtomicType::SC:
        atomic::scd(cpu, decoder);
        break;
    case AtomicType::XOR:
        atomic::amoxord(cpu, decoder);
        break;
    case AtomicType::OR:
        atomic::amoord(cpu, decoder);
        break;
    case AtomicType::AND:
        atomic::amoandd(cpu, decoder);
        break;
    case AtomicType::MIN:
        atomic::amomind(cpu, decoder);
        break;
    case AtomicType::MAX:
        atomic::amomaxd(cpu, decoder);
        break;
    case AtomicType::MINU:
        atomic::amominud(cpu, decoder);
        break;
    case AtomicType::MAXU:
        atomic::amomaxud(cpu, decoder);
        break;
    default:
        cpu.set_exception(exception::Exception::IllegalInstruction, decoder.insn);
        break;
    }
}

void atomic::funct3(Cpu& cpu, Decoder decoder)
{
    switch (decoder.funct3())
    {
    case AtomicType::AMOW:
        amow(cpu, decoder);
        break;
    case AtomicType::AMOD:
        amod(cpu, decoder);
        break;
    default:
        cpu.set_exception(exception::Exception::IllegalInstruction, decoder.insn);
        break;
    }
}

// amow

#define amotarget_t int32_t
#define amoutarget_t uint32_t

void atomic::amoaddw(Cpu& cpu, Decoder decoder)
{
    Cpu::reg_name rs1 = decoder.rs1();
    Cpu::reg_name rs2 = decoder.rs2();
    Cpu::reg_name rd = decoder.rd();

    uint64_t addres = cpu.regs[rs1];

    amotarget_t val1 = cpu.mmu.load(addres, 32);
    amotarget_t val2 = cpu.regs[rs2];

    amotarget_t result = val1 + val2;

    cpu.mmu.store(addres, result, 32);

    cpu.regs[rd] = SIGNEXTEND_CAST(val1, amotarget_t);
}

void atomic::amoswapw(Cpu& cpu, Decoder decoder)
{
    Cpu::reg_name rs1 = decoder.rs1();
    Cpu::reg_name rs2 = decoder.rs2();
    Cpu::reg_name rd = decoder.rd();

    uint64_t addres = cpu.regs[rs1];

    amotarget_t val1 = cpu.mmu.load(addres, 32);
    amotarget_t val2 = cpu.regs[rs2];

    cpu.mmu.store(addres, val2, 32);

    cpu.regs[rd] = SIGNEXTEND_CAST(val1, amotarget_t);
}

void atomic::lrw(Cpu& cpu, Decoder decoder)
{
    Cpu::reg_name rs1 = decoder.rs1();
    Cpu::reg_name rd = decoder.rd();

    uint64_t addres = cpu.regs[rs1];

    amotarget_t val1 = cpu.mmu.load(addres, 32);
    cpu.regs[rd] = SIGNEXTEND_CAST(val1, amotarget_t);

    cpu.reservations.insert(addres);
}

void atomic::scw(Cpu& cpu, Decoder decoder)
{
    Cpu::reg_name rs1 = decoder.rs1();
    Cpu::reg_name rs2 = decoder.rs2();
    Cpu::reg_name rd = decoder.rd();

    uint64_t addres = cpu.regs[rs1];

    amoutarget_t val2 = cpu.regs[rs2];

    if (cpu.reservations.contains(addres))
    {
        cpu.reservations.erase(addres);

        cpu.mmu.store(addres, val2, 32);

        cpu.regs[rd] = 0;
    }
    else
    {
        cpu.regs[rd] = 1;
    }
}

void atomic::amoxorw(Cpu& cpu, Decoder decoder)
{
    Cpu::reg_name rs1 = decoder.rs1();
    Cpu::reg_name rs2 = decoder.rs2();
    Cpu::reg_name rd = decoder.rd();

    uint64_t addres = cpu.regs[rs1];

    amotarget_t val1 = cpu.mmu.load(addres, 32);
    amotarget_t val2 = cpu.regs[rs2];

    cpu.mmu.store(addres, SIGNEXTEND_CAST(val1 ^ val2, amotarget_t), 32);

    cpu.regs[rd] = SIGNEXTEND_CAST(val1, amotarget_t);
}

void atomic::amoorw(Cpu& cpu, Decoder decoder)
{
    Cpu::reg_name rs1 = decoder.rs1();
    Cpu::reg_name rs2 = decoder.rs2();
    Cpu::reg_name rd = decoder.rd();

    uint64_t addres = cpu.regs[rs1];

    amotarget_t val1 = cpu.mmu.load(addres, 32);
    amotarget_t val2 = cpu.regs[rs2];

    cpu.mmu.store(addres, SIGNEXTEND_CAST(val1 | val2, amotarget_t), 32);

    cpu.regs[rd] = SIGNEXTEND_CAST(val1, amotarget_t);
}

void atomic::amoandw(Cpu& cpu, Decoder decoder)
{
    Cpu::reg_name rs1 = decoder.rs1();
    Cpu::reg_name rs2 = decoder.rs2();
    Cpu::reg_name rd = decoder.rd();

    uint64_t addres = cpu.regs[rs1];

    amotarget_t val1 = cpu.mmu.load(addres, 32);
    amotarget_t val2 = cpu.regs[rs2];

    cpu.mmu.store(addres, SIGNEXTEND_CAST(val1 & val2, amotarget_t), 32);

    cpu.regs[rd] = SIGNEXTEND_CAST(val1, amotarget_t);
}

void atomic::amominw(Cpu& cpu, Decoder decoder)
{
    Cpu::reg_name rs1 = decoder.rs1();
    Cpu::reg_name rs2 = decoder.rs2();
    Cpu::reg_name rd = decoder.rd();

    uint64_t addres = cpu.regs[rs1];

    amotarget_t val1 = cpu.mmu.load(addres, 32);
    amotarget_t val2 = cpu.regs[rs2];

    cpu.mmu.store(addres, SIGNEXTEND_CAST(std::min(val1, val2), amotarget_t), 32);

    cpu.regs[rd] = SIGNEXTEND_CAST(val1, amotarget_t);
}

void atomic::amomaxw(Cpu& cpu, Decoder decoder)
{
    Cpu::reg_name rs1 = decoder.rs1();
    Cpu::reg_name rs2 = decoder.rs2();
    Cpu::reg_name rd = decoder.rd();

    uint64_t addres = cpu.regs[rs1];

    amotarget_t val1 = cpu.mmu.load(addres, 32);
    amotarget_t val2 = cpu.regs[rs2];

    cpu.mmu.store(addres, SIGNEXTEND_CAST(std::max(val1, val2), amotarget_t), 32);

    cpu.regs[rd] = SIGNEXTEND_CAST(val1, amotarget_t);
}

void atomic::amominuw(Cpu& cpu, Decoder decoder)
{
    Cpu::reg_name rs1 = decoder.rs1();
    Cpu::reg_name rs2 = decoder.rs2();
    Cpu::reg_name rd = decoder.rd();

    uint64_t addres = cpu.regs[rs1];

    amoutarget_t val1 = cpu.mmu.load(addres, 32);
    amoutarget_t val2 = cpu.regs[rs2];

    cpu.mmu.store(addres, std::min(val1, val2), 32);

    cpu.regs[rd] = SIGNEXTEND_CAST(val1, amotarget_t);
}

void atomic::amomaxuw(Cpu& cpu, Decoder decoder)
{
    Cpu::reg_name rs1 = decoder.rs1();
    Cpu::reg_name rs2 = decoder.rs2();
    Cpu::reg_name rd = decoder.rd();

    uint64_t addres = cpu.regs[rs1];

    amoutarget_t val1 = cpu.mmu.load(addres, 32);
    amoutarget_t val2 = cpu.regs[rs2];

    cpu.mmu.store(addres, std::max(val1, val2), 32);

    cpu.regs[rd] = SIGNEXTEND_CAST(val1, amotarget_t);
}

#undef amotarget_t
#undef amoutarget_t

// amod

#define amotarget_t int64_t
#define amoutarget_t uint64_t

void atomic::amoaddd(Cpu& cpu, Decoder decoder)
{
    Cpu::reg_name rs1 = decoder.rs1();
    Cpu::reg_name rs2 = decoder.rs2();
    Cpu::reg_name rd = decoder.rd();

    uint64_t addres = cpu.regs[rs1];

    amotarget_t val1 = cpu.mmu.load(addres, 64);
    amotarget_t val2 = cpu.regs[rs2];

    amotarget_t result = val1 + val2;

    cpu.mmu.store(addres, result, 64);

    cpu.regs[rd] = SIGNEXTEND_CAST(val1, amotarget_t);
}

void atomic::amoswapd(Cpu& cpu, Decoder decoder)
{
    Cpu::reg_name rs1 = decoder.rs1();
    Cpu::reg_name rs2 = decoder.rs2();
    Cpu::reg_name rd = decoder.rd();

    uint64_t addres = cpu.regs[rs1];

    amotarget_t val1 = cpu.mmu.load(addres, 64);
    amotarget_t val2 = cpu.regs[rs2];

    cpu.mmu.store(addres, val2, 64);

    cpu.regs[rd] = SIGNEXTEND_CAST(val1, amotarget_t);
}

void atomic::lrd(Cpu& cpu, Decoder decoder)
{
    Cpu::reg_name rs1 = decoder.rs1();
    Cpu::reg_name rd = decoder.rd();

    uint64_t addres = cpu.regs[rs1];

    amotarget_t val1 = cpu.mmu.load(addres, 64);
    cpu.regs[rd] = SIGNEXTEND_CAST(val1, amotarget_t);

    cpu.reservations.insert(addres);
}

void atomic::scd(Cpu& cpu, Decoder decoder)
{
    Cpu::reg_name rs1 = decoder.rs1();
    Cpu::reg_name rs2 = decoder.rs2();
    Cpu::reg_name rd = decoder.rd();

    uint64_t addres = cpu.regs[rs1];

    amoutarget_t val2 = cpu.regs[rs2];

    if (cpu.reservations.contains(addres))
    {
        cpu.reservations.erase(addres);

        cpu.mmu.store(addres, val2, 64);

        cpu.regs[rd] = 0;
    }
    else
    {
        cpu.regs[rd] = 1;
    }
}

void atomic::amoxord(Cpu& cpu, Decoder decoder)
{
    Cpu::reg_name rs1 = decoder.rs1();
    Cpu::reg_name rs2 = decoder.rs2();
    Cpu::reg_name rd = decoder.rd();

    uint64_t addres = cpu.regs[rs1];

    amotarget_t val1 = cpu.mmu.load(addres, 64);
    amotarget_t val2 = cpu.regs[rs2];

    cpu.mmu.store(addres, SIGNEXTEND_CAST(val1 ^ val2, amotarget_t), 64);

    cpu.regs[rd] = SIGNEXTEND_CAST(val1, amotarget_t);
}

void atomic::amoord(Cpu& cpu, Decoder decoder)
{
    Cpu::reg_name rs1 = decoder.rs1();
    Cpu::reg_name rs2 = decoder.rs2();
    Cpu::reg_name rd = decoder.rd();

    uint64_t addres = cpu.regs[rs1];

    amotarget_t val1 = cpu.mmu.load(addres, 64);
    amotarget_t val2 = cpu.regs[rs2];

    cpu.mmu.store(addres, SIGNEXTEND_CAST(val1 | val2, amotarget_t), 64);

    cpu.regs[rd] = SIGNEXTEND_CAST(val1, amotarget_t);
}

void atomic::amoandd(Cpu& cpu, Decoder decoder)
{
    Cpu::reg_name rs1 = decoder.rs1();
    Cpu::reg_name rs2 = decoder.rs2();
    Cpu::reg_name rd = decoder.rd();

    uint64_t addres = cpu.regs[rs1];

    amotarget_t val1 = cpu.mmu.load(addres, 64);
    amotarget_t val2 = cpu.regs[rs2];

    cpu.mmu.store(addres, SIGNEXTEND_CAST(val1 & val2, amotarget_t), 64);

    cpu.regs[rd] = SIGNEXTEND_CAST(val1, amotarget_t);
}

void atomic::amomind(Cpu& cpu, Decoder decoder)
{
    Cpu::reg_name rs1 = decoder.rs1();
    Cpu::reg_name rs2 = decoder.rs2();
    Cpu::reg_name rd = decoder.rd();

    uint64_t addres = cpu.regs[rs1];

    amotarget_t val1 = cpu.mmu.load(addres, 64);
    amotarget_t val2 = cpu.regs[rs2];

    cpu.mmu.store(addres, SIGNEXTEND_CAST(std::min(val1, val2), amotarget_t), 64);

    cpu.regs[rd] = SIGNEXTEND_CAST(val1, amotarget_t);
}

void atomic::amomaxd(Cpu& cpu, Decoder decoder)
{
    Cpu::reg_name rs1 = decoder.rs1();
    Cpu::reg_name rs2 = decoder.rs2();
    Cpu::reg_name rd = decoder.rd();

    uint64_t addres = cpu.regs[rs1];

    amotarget_t val1 = cpu.mmu.load(addres, 64);
    amotarget_t val2 = cpu.regs[rs2];

    cpu.mmu.store(addres, SIGNEXTEND_CAST(std::max(val1, val2), amotarget_t), 64);

    cpu.regs[rd] = SIGNEXTEND_CAST(val1, amotarget_t);
}

void atomic::amominud(Cpu& cpu, Decoder decoder)
{
    Cpu::reg_name rs1 = decoder.rs1();
    Cpu::reg_name rs2 = decoder.rs2();
    Cpu::reg_name rd = decoder.rd();

    uint64_t addres = cpu.regs[rs1];

    amoutarget_t val1 = cpu.mmu.load(addres, 64);
    amoutarget_t val2 = cpu.regs[rs2];

    cpu.mmu.store(addres, SIGNEXTEND_CAST(std::min(val1, val2), amoutarget_t), 64);

    cpu.regs[rd] = SIGNEXTEND_CAST(val1, amoutarget_t);
}

void atomic::amomaxud(Cpu& cpu, Decoder decoder)
{
    Cpu::reg_name rs1 = decoder.rs1();
    Cpu::reg_name rs2 = decoder.rs2();
    Cpu::reg_name rd = decoder.rd();

    uint64_t addres = cpu.regs[rs1];

    amoutarget_t val1 = cpu.mmu.load(addres, 64);
    amoutarget_t val2 = cpu.regs[rs2];

    cpu.mmu.store(addres, SIGNEXTEND_CAST(std::max(val1, val2), amoutarget_t), 64);

    cpu.regs[rd] = SIGNEXTEND_CAST(val1, amoutarget_t);
}

#undef amotarget_t
#undef amoutarget_t