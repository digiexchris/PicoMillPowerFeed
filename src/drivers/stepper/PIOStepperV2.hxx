#pragma once
#include "PIOStepper.hxx"
#include "Settings.hxx"
#include "Stepper.hxx"
#include "hardware/clocks.h"
#include "hardware/pio.h"
#include <memory>

namespace PowerFeed::Drivers
{
	class PIOStepperV2 : public IStepper
	{
	public:
		PIOStepperV2(std::shared_ptr<SettingsManager> aSettings, PIO pio, uint sm);

		void SetAcceleration(uint32_t acceleration) override;
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
		void SetDeceleration(uint32_t deceleration);

	private:
		void PrivInit(float div, int steps_per_rotation, int32_t acceleration, int32_t deceleration);
		void PrivUpdate();
		static void PrivUpdateTask(void *pvParameters);

		std::shared_ptr<SettingsManager> mySettingsManager;
		std::unique_ptr<PIOStepper::PIOStepper> myPIOStepper;

		float myTargetHz;
		bool myIsRunning;
		bool myIsEnabled;
		bool myDirection;
		bool myTargetDirection;
		// Remove myCurrentPeriod from here

		static constexpr double PI = 3.14159265358979323846;
	};
}