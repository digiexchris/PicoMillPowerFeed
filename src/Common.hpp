#pragma once

#include <stdint.h>

#include <chrono>

namespace PicoMill
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

}