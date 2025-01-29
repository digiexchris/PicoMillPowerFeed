#include "Encoder.hpp"
#include <sys/_stdint.h>

int32_t Encoder::GetAndResetDelta()
{
	int32_t delta = myDelta;
	myDelta = 0;
	return delta;
}

void Encoder::IncrementDelta(int8_t aDelta)
{
	myDelta += aDelta;
}