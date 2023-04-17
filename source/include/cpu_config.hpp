#pragma once

#ifndef CPU_VERBOSE_DEBUG
#define CPU_VERBOSE_DEBUG 0
#endif

#ifndef NATIVE_CLI
#define NATIVE_CLI 0
#endif

#ifndef USE_TLB
#define USE_TLB 1
#endif

#if USE_TLB && !defined(TLB_COMPLIANT)
// Performance slightly better when disabled.
// When disabled, the dirty bit will only be set if the PTE is not
// available in the TLB cache and a store access is being performed.
// If the PTE is cached before the store operation occurs,
// the dirty bit will not be set.
#define TLB_COMPLIANT 1
#else
#define TLB_COMPLIANT 0
#endif
