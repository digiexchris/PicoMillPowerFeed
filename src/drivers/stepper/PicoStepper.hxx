#pragma once
#include "Common.hxx"
#include "FreeRTOS.h"
#include "Settings.hxx"
#include "Stepper.hxx"
#include "hardware/clocks.h"
#include "hardware/pio.h"
#include "task.h"
#include <PIOStepperSpeedController/PIOStepper.hxx>
#include <PIOStepperSpeedController/Stepper.hxx>

namespace PowerFeed::Drivers
{
	class PicoStepper;
	using DefaultStepperType = PicoStepper;

	class PicoStepper : public Stepper<PicoStepper>
	{
	public:
		PicoStepper(SettingsManager *aSettings, Time *aTime, PIO pio, uint sm);
		~PicoStepper();

		void SetDirection(bool direction);
		bool GetDirection();
		bool GetTargetDirection();
		uint32_t GetTargetSpeed();
		void SetSpeed(uint32_t speed);
		uint32_t GetCurrentSpeed();
		void Start();
		void Stop();
		bool IsRunning();

	private:
		void PrivUpdate();
		static void PrivUpdateTask(void *pvParameters);

		SettingsManager *mySettingsManager;
		PIOStepperSpeedController::PIOStepper *myPIOStepper;
		Time *myTime;
		uint64_t myStoppedAt;
		bool myDirection;
		bool myTargetDirection;
		bool myIsEnabled;
		TaskHandle_t myTaskHandle;
	};
}