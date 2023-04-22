#pragma once

#ifndef CPU_VERBOSE_DEBUG
#define CPU_VERBOSE_DEBUG 1
#endif

#ifndef NATIVE_CLI
#define NATIVE_CLI 0
#endif

#ifndef USE_TLB
#define USE_TLB 1
#endif

#if USE_TLB && !defined(TLB_COMPLIANT)
// Translation performance is better when disabled.
// When disabled, MMU safety checks will only be performed
// on TLB cache miss, but not if the entry is already in the cache.
#define TLB_COMPLIANT 1
#else
// Chooses the desired code path. When TLB is disabled translation is still compliant regardless of
// the TLB_COMPLIANT value.
#define TLB_COMPLIANT 0
#endif
