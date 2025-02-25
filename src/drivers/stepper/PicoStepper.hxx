#pragma once
#include "Settings.hxx"
#include "Stepper.hxx"
#include "hardware/clocks.h"
#include "hardware/pio.h"
#include <PIOStepperSpeedController/PIOStepper.hxx>
#include <PIOStepperSpeedController/Stepper.hxx>
#include <memory>

namespace PowerFeed::Drivers
{
	class PicoStepper : public Stepper<PicoStepper>
	{
	public:
		PicoStepper(std::shared_ptr<SettingsManager> aSettings, PIO pio, uint sm);

		void SetDirection(bool direction);
		bool GetDirection();
		bool GetTargetDirection();
		uint32_t GetTargetSpeed();
		void SetSpeed(uint32_t speed);
		void Init();
		uint32_t GetCurrentSpeed();
		void Start();
		void Stop();

	private:
		void PrivUpdate();
		static void PrivUpdateTask(void *pvParameters);

		std::shared_ptr<SettingsManager> mySettingsManager;
		std::unique_ptr<PIOStepperSpeedController::PIOStepper> myPIOStepper;

		bool myDirection;
		bool myTargetDirection;
	};
}