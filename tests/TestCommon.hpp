#include "../src/Common.hpp"
#include <chrono>
#include <gmock/gmock.h>

class TestTime : public ITime
{
public:
	MOCK_METHOD(uint64_t, GetCurrentTimeInMilliseconds, (), (const, override));
};