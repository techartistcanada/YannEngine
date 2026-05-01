#pragma once

// ============================================
// RHI Backend Selection (compile-time switch)
// Uncomment exactly one of the following:
// ============================================

#define USE_DX11
//#define USE_DX12

// Validation
#if defined(USE_DX11) && defined(USE_DX12)
    #error "Cannot enable both USE_DX11 and USE_DX12 at the same time."
#endif
#if !defined(USE_DX11) && !defined(USE_DX12)
    #error "Must enable either USE_DX11 or USE_DX12."
#endif