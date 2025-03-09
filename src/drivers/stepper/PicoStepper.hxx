#pragma once
#include "Common.hxx"
#include "FreeRTOS.h"
#include "Settings.hxx"
#include "Stepper.hxx"
#include "hardware/clocks.h"
#include "hardware/pio.h"
#include "task.h"
#include <Mutex.hxx>
#include <PIOStepperSpeedController/PIOStepper.hxx>
#include <PIOStepperSpeedController/Stepper.hxx>
#include <semphr.h>
#include <stdint.h>
#include <time.h>

namespace PowerFeed::Drivers
{

	class PicoStepper : public StepperBase<PicoStepper>
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
		bool IsStopping();

	private:
		void PrivUpdate();
		static void PrivUpdateTask(void *pvParameters);
		void PrivEnable();
		void PrivDisable();

		SettingsManager *mySettingsManager;
		PIOStepperSpeedController::PIOStepper *myPIOStepper;
		Time *myTime;
		uint64_t myStoppedAt;
		bool myDirection;
		bool myTargetDirection;
		bool myIsEnabled;
		TaskHandle_t myTaskHandle;
		Mutex myMutex;
	};

	static_assert(PowerFeed::StepperImpl<PicoStepper>,
				  "PicoStepper must implement all required methods");

	using DefaultStepperType = PowerFeed::Stepper<PicoStepper>;
}