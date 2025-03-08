#pragma once

#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>

#include <pico/platform/panic.h>
#include <pico.h>


#define BREAKPOINT() __asm volatile(".syntax unified\nbkpt #0")


#ifdef __cplusplus
extern "C" {
#endif

inline void Panic(const char *message, ...)
{
	va_list args;
	va_start(args, message);
	// Now args contains the variadic arguments that we'll pass to panic
	BREAKPOINT();
	panic(message, args);
}






#define ASSERT(condition)  Assert(condition)

inline void Assert(bool condition)
{
	if (!condition)
	{
		BREAKPOINT();
		panic("ASSERT FAILED: %s, %u",  __FILE__, __LINE__);
	}
}


#ifdef __cplusplus
}
#endif