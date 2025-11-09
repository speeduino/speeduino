#pragma once

#if !defined(ATOMIC)

#if __has_include(<SimplyAtomic.h>)
  #include <SimplyAtomic.h>
#elif __has_include(<util/atomic.h>)
  //Fallback for Arduino IDE when SimplyAtomic is not installed
  #include <util/atomic.h>
  #define ATOMIC() ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
  #warning It is strongly recommended to install the SimplyAtomic library rather than relying on the built-in ATOMIC
#else
  #define ATOMIC()
  #warning ATOMIC() macro is empty - no atomic operations are in effect
#endif

#endif