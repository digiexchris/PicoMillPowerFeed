#pragma once

#include <stdint.h>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
#pragma clang diagnostic ignored "-Wall"
// NOLINT(misc-include)
#include <chrono>
#pragma clang diagnostic pop

namespace PowerFeed
{

	class ITime
	{
	public:
		virtual uint64_t GetCurrentTimeInMilliseconds() const = 0;
	};

	class Time : public ITime
	{
	public:
		virtual uint64_t GetCurrentTimeInMilliseconds() const override;
	};

	extern "C" void BreakPanic(const char *message);

}