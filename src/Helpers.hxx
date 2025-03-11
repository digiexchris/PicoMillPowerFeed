#pragma once

#ifdef UNIT_TEST
#define configTICK_RATE_HZ 1000
static inline void ExitTask()
{
	// vTaskDelete(nullptr);
}
#else
#include <FreeRTOS.h>

static inline void ExitTask()
{
	vTaskDelete(nullptr);
}
#endif

inline double ScaleValue(double value, double minOld, double maxOld, double minNew, double maxNew)
{
	return (value - minOld) / (maxOld - minOld) * (maxNew - minNew) + minNew;
}

template <typename T>
T abs(T value)
{
	return static_cast<T>(value < 0 ? -value : value);
}

template <typename T, typename T2>
T min(T a, T2 b)
{
	return static_cast<T>(a < b ? a : b);
}

#define MS_TO_TICKS(ms) (ms * configTICK_RATE_HZ / 1000)

#define MS_TO_US(ms) (ms * 1000)

template <typename T>
bool IsWithinRange(T value, T compareTo, T deadzone)
{
	return abs(value - compareTo) <= deadzone;
}