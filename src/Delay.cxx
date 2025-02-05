// #include <algorithm>
#include <cmath>
#include <cstdint>
#include <stdint.h>

namespace PicoMill
{

	constexpr double PI = 3.14159265358979323846;

	/**
		@brief Calculate the time interval between the previous step and this step
		@param stepsPerRotation The number of steps per rotation of the stepper
	   motor
		@param acceleration The acceleration of the stepper motor in steps per
	   second squared
		@param previousInterval The previous interval in seconds
		@return The next interval in seconds
	 */
	uint32_t CalculateNextInterval(int stepsPerRotation, uint32_t currentSpeed, int32_t acceleration)
	{
		uint32_t previousInterval = 1 / currentSpeed / 1000000;

		double alpha = 2 * PI / stepsPerRotation; // Step angle in radians

		// Convert acceleration from steps/s^2 to rad/s^2
		double radsPerSecondSquared = acceleration * alpha;

		// If previousInterval is hard-coded, assume first interval is provided
		// correctly
		if (previousInterval <= 0)
		{
			return std::sqrt(2 * alpha /
							 acceleration); // Use original acceleration in steps/s^2
		}

		// Calculate subsequent intervals
		return previousInterval -
			   (2 * previousInterval * previousInterval * radsPerSecondSquared) /
				   alpha;
	}

} // namespace PicoMill