#include "PicoStepper.hxx"
#include <FreeRTOS.h>
#include <PIOStepper.hxx>
#include <task.h>

namespace PowerFeed::Drivers
{
	PicoStepper::PicoStepper(std::shared_ptr<SettingsManager> aSettings, PIO pio, uint sm)
		: mySettingsManager(aSettings), myIsRunning(false)
	{
		Settings::Driver driver = mySettingsManager->Get()->driver;

		uint sysclk = clock_get_hz(clk_sys);

		// Initialize
		Settings::Mechanical mech = mySettingsManager->Get()->mechanical;

		myPIOStepper = std::make_unique<PIOStepperSpeedController::PIOStepper>(driver.driverStepPin, 10, mech.maxStepsPerSecond, mech.acceleration, mech.deceleration, sysclk, 125, nullptr, nullptr, nullptr, nullptr);
	}

	void PicoStepper::Init()
	{
		xTaskCreate(PrivUpdateTask, "StepperV2", 2048, this, 15, NULL);
	}

	void PicoStepper::SetSpeed(uint32_t speed)
	{
		myPIOStepper->SetTargetHz(speed);
	}

	void PicoStepper::PrivUpdate()
	{
		if (!myPIOStepper->Update())
		{
			vTaskDelay(pdMS_TO_TICKS(5));
		}
	}

	void PicoStepper::Disable()
	{
		panic("Not implemented, needs to check that the PIOStepper isn't running first.");
		gpio_put(mySettingsManager->Get()->driver.driverEnPin,
				 mySettingsManager->Get()->driver.driverDisableValue);
	}

	void PicoStepper::Enable()
	{
		gpio_put(mySettingsManager->Get()->driver.driverEnPin,
				 mySettingsManager->Get()->driver.driverEnableValue);
		vTaskDelay(pdMS_TO_TICKS(mySettingsManager->Get()->driver.driverDirectionChangeDelayMs));
		myPIOStepper->Start();
	}

	void PicoStepper::PrivUpdateTask(void *pvParameters)
	{
		auto stepper = static_cast<PicoStepper *>(pvParameters);
		while (true)
		{
			stepper->PrivUpdate();
		}
	}

	bool PicoStepper::GetDirection() { return myDirection; }
	bool PicoStepper::GetTargetDirection() { return myTargetDirection; }
	uint32_t PicoStepper::GetTargetSpeed()
	{
		auto stepperState = myPIOStepper->GetState();
		if (stepperState == PIOStepperSpeedController::StepperState::STOPPED || stepperState == PIOStepperSpeedController::StepperState::STOPPING)
		{
			return 0;
		}
		{
			return 0;
		}
		myPIOStepper->GetTargetFrequency();
	}
	uint32_t PicoStepper::GetCurrentSpeed()
	{
		auto state = myPIOStepper->GetState();
		if (state == PIOStepperSpeedController::StepperState::STOPPED)
		{
			return 0;
		}
		return myPIOStepper->GetCurrentFrequency();
	}
	bool PicoStepper::IsEnabled() { return myIsEnabled; }
	void PicoStepper::SetDirection(bool direction)
	{
		panic("Not implemented, need to implement checking to make myPIOStepper is stopped before changing direction");
		myTargetDirection = direction;
		gpio_put(mySettingsManager->Get()->driver.driverDirPin, direction);
		myDirection = direction;
		vTaskDelay(pdMS_TO_TICKS(mySettingsManager->Get()->driver.driverDirectionChangeDelayMs));
	}
}
