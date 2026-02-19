/*
 * addon_api.h — Unified abstraction for NAN and N-API backends
 *
 * Include this instead of <nan.h> or <napi.h>. The backend is selected
 * at compile time via USE_NAN or USE_NAPI defines (set in CMakeLists.txt).
 * Default: NAN (backward compatible).
 */

#pragma once

#if defined(USE_NAPI)
  #include "addon_api_napi.h"
#else
  // Default to NAN for backward compatibility
  #ifndef USE_NAN
    #define USE_NAN 1
  #endif
  #include "addon_api_nan.h"
#endif
