#pragma once

#include <stdint.h>

namespace PicoMill
{
	/**
		@brief Calculate the time interval between the previous step and this step
		@param stepsPerRotation The number of steps per rotation of the stepper
		motor
		@param acceleration The acceleration of the stepper motor in steps per
		second squared
		@param previousInterval The previous interval in seconds
		@return The next interval in seconds
	 */
	uint32_t CalculateNextInterval(int stepsPerRotation, uint32_t currentSpeed, int32_t acceleration);

}