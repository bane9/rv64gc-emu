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
// When disabled, MMU safety checks will only be performed
// only at the time the page is being fetched, but not if
// it's already cached. As a result, unpredictable behavior
// may occur with invalid page tables,
// and it presents a security risk.
#define TLB_COMPLIANT 1
#else
#define TLB_COMPLIANT 0
#endif
