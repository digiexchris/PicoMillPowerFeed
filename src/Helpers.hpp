#pragma once

#ifdef UNIT_TEST
#define configTICK_RATE_HZ 1000
#else
// #include <FreeRTOS.h>
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

template <typename T>
T floor(T value)
{
	return static_cast<T>(static_cast<long long>(value));
}

//#define MS_TO_TICKS(ms) (ms * configTICK_RATE_HZ / 1000)

#define MS_TO_US(ms) (ms * 1000)

template <typename T>
bool IsWithinRange(T value, T compareTo, T deadzone)
{
	return abs(value - compareTo) <= deadzone;
}

inline float MMToInch(float mm)
{
	return mm / 25.4;
}

inline float InchToMM(float inch)
{
	return inch * 25.4;
}