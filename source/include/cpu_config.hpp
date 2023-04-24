#pragma once

#ifndef CPU_VERBOSE_DEBUG
#define CPU_VERBOSE_DEBUG 0
#endif

#ifndef NATIVE_CLI
#define NATIVE_CLI 0
#endif

#ifndef USE_TLB
#define USE_TLB 0
#endif

#ifndef DRAM_BASE
#define DRAM_BASE 0x80000000U
#endif

#ifndef KERNEL_OFFSET
#define KERNEL_OFFSET 0x20000U
#endif

#if USE_TLB && !defined(TLB_COMPLIANT)
// Translation performance is better when disabled.
// When disabled, MMU safety checks will only be performed
// on TLB cache miss, but not if the entry is already in the cache.
#define TLB_COMPLIANT 0
#else
// Chooses the desired code path. When TLB is disabled translation is still compliant regardless of
// the TLB_COMPLIANT value.
#define TLB_COMPLIANT 0
#endif
