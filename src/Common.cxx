#include "Common.hxx"
#include <chrono>
#include <pico.h>

namespace PowerFeed
{

	uint64_t Time::GetCurrentTimeInMilliseconds() const
	{
		return std::chrono::duration_cast<std::chrono::milliseconds>(
				   std::chrono::high_resolution_clock::now().time_since_epoch())
			.count();
	}

	void BreakPanic(const char *message)
	{
		__breakpoint();
		panic(message);
	}

} // namespace