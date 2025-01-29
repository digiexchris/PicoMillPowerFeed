#pragma once

#include <cstdint>
class Encoder
{
public:
	int32_t GetAndResetDelta();

	void IncrementDelta(int8_t aDelta);

private:
	int32_t myDelta = 0;
};