#include "../src/Common.hxx"
#include <chrono>
#include <gmock/gmock.h>

using namespace PowerFeed;

class TestTime : public ITime
{
public:
	MOCK_METHOD(uint64_t, GetCurrentTimeInMilliseconds, (), (const, override));
};