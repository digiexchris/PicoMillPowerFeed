#pragma once

#include "Settings.hxx"
#include "Stepper.hxx"
#include "hardware/pio.h" // Include the header file that defines PIO
#include <memory>

namespace PowerFeed::Drivers
{
	class PIOStepper : public IStepper
	{
	public:
		/**
		 * @brief Construct a new Stepper object
		 *
		 * @param pio The PIO instance to use
		 * @param sm The PIO state machine to use
		 */

		PIOStepper(std::shared_ptr<SettingsManager> aSettings, PIO pio, uint sm);

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

		std::shared_ptr<SettingsManager> mySettingsManager;

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

} // namespace ::Drivers