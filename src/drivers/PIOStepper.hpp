#pragma once

#include "Stepper.hpp"
#include "hardware/pio.h" // Include the header file that defines PIO

namespace PicoMill::Drivers
{
	class PIOStepper : public IStepper
	{
	public:
		/**
		 * @brief Construct a new Stepper object
		 *
		 * @param stepPin The GPIO pin number for the step signal
		 * @param dirPin The GPIO pin number for the direction signal
		 * @param enPin The GPIO pin number for the enable signal
		 * @param targetSpeed The initial target speed in steps per second
		 * @param acceleration The acceleration in steps per second squared
		 * @param pio The PIO instance to use
		 * @param sm The PIO state machine to use
		 */
		PIOStepper(uint stepPin, uint dirPin, uint enPin, uint32_t maxSpeed, uint32_t acceleration, uint32_t decelerationMultiplier, PIO pio, uint sm, uint16_t stepsPerRev);

		void SetDirection(bool direction) override;
		bool GetDirection() override;
		bool GetTargetDirection() override;
		uint32_t GetTargetSpeed() override;
		void SetSpeed(uint32_t speed) override;
		uint32_t GetSetSpeed() override;
		void Start() override;
		uint32_t GetCurrentSpeed() override;
		void Enable() override;
		void Disable() override;
		bool IsEnabled() override;
		void Update();
		void SetAcceleration(uint32_t acceleration) override;

	private:
		static void PIOStepperUpdateStepperTask(void *pvParameters);

		void WriteToStepper(uint32_t delay);
		void DirectionChangedWait();

		bool HandleDirection();

		uint stepPin;
		uint dirPin;
		uint enPin;

		bool enabled = false;

		PIO pio;
		uint sm;
		bool myDirection;
		bool myTargetDirection;

		uint32_t myAcceleration;
		uint32_t myDecelerationMultiplier;

		uint16_t myStepsPerRev;

		uint32_t myCurrentSpeed = 0; // in steps per second
		uint32_t myTargetSpeed = 0;	 // in steps per second
	};

} // namespace PicoMill::Drivers