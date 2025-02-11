#include "../src/Common.hxx"
#include <chrono>
#include <gmock/gmock.h>

using namespace PowerFeed;

inline void panic(const char *msg)
{
	printf("Panic: %s\n", msg);
	assert(false);
}

class TestTime : public ITime
{
public:
	MOCK_METHOD(uint64_t, GetCurrentTimeInMilliseconds, (), (const, override));
};