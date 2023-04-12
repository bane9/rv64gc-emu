#pragma once
#include <cstdint>

#ifndef CPU_VERBOSE_DEBUG
#define CPU_VERBOSE_DEBUG 0
#endif

#ifndef NATIVE_CLI
#define NATIVE_CLI 0
#endif

constexpr uint64_t clint_base_addr = 0x2000000ULL;
constexpr uint64_t plic_base_addr = 0xC000000ULL;

constexpr uint64_t uart_irqn = 10;
