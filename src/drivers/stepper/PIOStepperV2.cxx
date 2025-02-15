#include "PIOStepperV2.hxx"
#include "Delay.hxx"
#include "stepperDriverV2.pio.h"
#include <FreeRTOS.h>
#include <task.h>

namespace PowerFeed::Drivers
{
	PIOStepperV2::PIOStepperV2(std::shared_ptr<SettingsManager> aSettings, PIO pio, uint sm)
		: mySettingsManager(aSettings), myIsRunning(false), myIsEnabled(false)
	{
		Settings::Driver driver = mySettingsManager->Get()->driver;

		// Initialize direction and enable pins
		gpio_init(driver.driverDirPin);
		gpio_set_dir(driver.driverDirPin, GPIO_OUT);
		gpio_init(driver.driverEnPin);
		gpio_set_dir(driver.driverEnPin, GPIO_OUT);

		// Create PIOStepper instance
		myPIOStepper = std::make_unique<PIOStepper::PIOStepper>(pio, sm, driver.driverStepPin);

		// Initialize
		Settings::Mechanical mech = mySettingsManager->Get()->mechanical;
		PrivInit(125.0f,
				 mech.stepsPerMotorRev,
				 mech.acceleration,
				 mech.deceleration); // Initialize with same accel/decel
	}

	void PIOStepperV2::PrivInit(float div, int steps_per_rotation, int32_t acceleration, int32_t deceleration)
	{
	}

	void PIOStepperV2::PrivStart(float target_hz)
	{
		myTargetHz = target_hz;

		if (!myIsRunning)
		{
			myPIOStepper->Start(myTargetHz);
			myIsRunning = true;
		}
	}

	bool PIOStepperV2::PrivStep()
	{
		// Calculate next period based on current PIOStepper state
		if (!myPIOStepper->Step())
		{
			return false;
		}
		return true;
	}

	// Interface methods now delegate to PIO SDK-style methods
	void PIOStepperV2::Start()
	{
		xTaskCreate(PrivUpdateTask, "StepperV2", 2048, this, 15, NULL);
	}

	void PIOStepperV2::SetSpeed(uint32_t speed)
	{
		myTargetHz = speed;
	}

	void PIOStepperV2::PrivUpdate()
	{
		if (!PrivStep())
		{
			xTaskDelay(pdMS_TO_TICKS(5));
		}
	}

	void PIOStepperV2::Disable()
	{
		gpio_put(mySettingsManager->Get()->driver.driverEnPin,
				 mySettingsManager->Get()->driver.driverDisableValue);
	}

	void PIOStepperV2::SetAcceleration(uint32_t acceleration)
	{
		// myPIOStepper->SetAcceleration(acceleration);
		panic("Not implemented");
	}

	void PIOStepperV2::SetDeceleration(uint32_t deceleration)
	{
		// myPIOStepper->SetDeceleration(deceleration);
		panic("Not implemented");
	}

	void PIOStepperV2::Enable()
	{
		gpio_put(mySettingsManager->Get()->driver.driverEnPin,
				 mySettingsManager->Get()->driver.driverEnableValue);
		vTaskDelay(pdMS_TO_TICKS(mySettingsManager->Get()->driver.driverDirectionChangeDelayMs));
	}

	void PIOStepperV2::PrivUpdateTask(void *pvParameters)
	{
		auto stepper = static_cast<PIOStepperV2 *>(pvParameters);
		while (true)
		{
			stepper->PrivUpdate();
			vTaskDelay(pdMS_TO_TICKS(5));
		}
	}

	// Implement remaining interface methods
	bool PIOStepperV2::GetDirection() { return myPIOStepper->GetDirection(); }
	bool PIOStepperV2::GetTargetDirection() { return myPIOStepper->GetTargetDirection(); }
	uint32_t PIOStepperV2::GetTargetSpeed() { return myTargetHz; }
	uint32_t PIOStepperV2::GetSetSpeed() { return myTargetHz; }
	uint32_t PIOStepperV2::GetCurrentSpeed() { return myPIOStepper->GetCurrentSpeed(); }
	bool PIOStepperV2::IsEnabled() { return myIsEnabled; }
	void PIOStepperV2::SetDirection(bool direction)
	{
		myPIOStepper->SetDirection(direction);
		myTargetDirection = direction;
		gpio_put(mySettingsManager->Get()->driver.driverDirPin, direction);
		myDirection = direction;
		vTaskDelay(pdMS_TO_TICKS(mySettingsManager->Get()->driver.driverDirDelay));
	}
}
