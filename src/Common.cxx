#include "Common.hxx"
// #include <chrono>

namespace PowerFeed
{

	uint64_t Time::GetCurrentTimeInMilliseconds() const
	{
		// 	return std::chrono::duration_cast<std::chrono::milliseconds>(
		// 			   std::chrono::high_resolution_clock::now().time_since_epoch())
		// 		.count();
		return 0;
	}

} // namespace PowerFeed