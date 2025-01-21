#pragma once

#include <stdint.h>

#include <chrono>

inline uint64_t getCurrentTimeInMilliseconds()
{
	return std::chrono::duration_cast<std::chrono::milliseconds>(
			   std::chrono::high_resolution_clock::now().time_since_epoch())
		.count();
}