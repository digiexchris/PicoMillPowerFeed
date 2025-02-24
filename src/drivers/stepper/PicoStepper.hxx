#pragma once
#include "Settings.hxx"
#include "hardware/clocks.h"
#include "hardware/pio.h"
#include <PIOStepper.hxx>
#include <Stepper.hxx>
#include <memory>

namespace PowerFeed::Drivers
{
	class PicoStepper : public IStepper<PicoStepper>
	{
	public:
		PicoStepper(std::shared_ptr<SettingsManager> aSettings, PIO pio, uint sm);

		void SetDirection(bool direction);
		bool GetDirection() override;
		bool GetTargetDirection() override;
		uint32_t GetTargetSpeed() override;
		void SetSpeed(uint32_t speed) override;
		void Init();
		uint32_t GetCurrentSpeed() override;
		void Enable() override;
		void Disable() override;
		bool IsEnabled() override;

	private:
		void PrivUpdate();
		static void PrivUpdateTask(void *pvParameters);

		std::shared_ptr<SettingsManager> mySettingsManager;
		std::unique_ptr<PIOStepperSpeedController::PIOStepper> myPIOStepper;

		bool myIsEnabled;
		bool myDirection;
		bool myTargetDirection;
	};
}