#include "PIOStepper.hxx"

#include "Delay.hxx"
#include "Helpers.hxx"
#include "build/src/stepper.pio.h"
#include "config.h"
#include "pico/time.h"
#include "stepper.pio.h"
#include <FreeRTOS.h>
#include <cmath>
#include <cstdint>
#include <exception>
#include <pico/printf.h>
#include <stdexcept>
#include <stdio.h>
#include <task.h>

namespace PowerFeed::Drivers
{
	PIOStepper::PIOStepper(std::shared_ptr<SettingsManager> aSettings, PIO pio, uint sm) : mySettingsManager(aSettings)
	{

		Settings::Driver driver = mySettingsManager->Get()->driver;
		gpio_init(driver.driverDirPin);
		gpio_set_dir(driver.driverDirPin, GPIO_OUT);
		gpio_init(driver.driverEnPin);
		gpio_set_dir(driver.driverEnPin, GPIO_OUT);
		uint offset = pio_add_program(pio, (const pio_program_t *)&simplestepper_program);
		pio_sm_config c = simplestepper_program_get_default_config(offset);
		sm_config_set_sideset_pins(&c, driver.driverStepPin);					// Configure the step pin for side-set operations
		sm_config_set_set_pins(&c, driver.driverStepPin, 1);					// Configure the step pin for set operations
		pio_gpio_init(pio, driver.driverStepPin);								// Initialize the GPIO pin
		pio_sm_set_consecutive_pindirs(pio, sm, driver.driverStepPin, 1, true); // Set the GPIO direction to output

		float clockDiv = 125;
		sm_config_set_clkdiv(&c, clockDiv); // Set the clock divider

		pio_sm_init(pio, sm, offset, &c);  // Initialize the state machine
		pio_sm_set_enabled(pio, sm, true); // Enable the state machine
	}

	void PIOStepper::Start()
	{
		Disable();

		xTaskCreate(PIOStepperUpdateStepperTask, "Stepper Task", 2048, this, 15, NULL);

		Enable();
	}

	void PIOStepper::SetAcceleration(uint32_t acceleration)
	{
		myAcceleration = acceleration;
	}

	void PIOStepper::SetDirection(bool direction)
	{
		if (this->myTargetDirection == direction)
		{
			return;
		}

		this->myTargetDirection = direction;
	}

	void PIOStepper::DirectionChangedWait()
	{
		vTaskDelay(MS_TO_TICKS(mySettingsManager->Get()->driver.driverDirectionChangeDelayMs));
	}

	void PIOStepper::WriteToStepper(uint32_t aDelay)
	{
		// todo since this is speed based instead of position based, we should just update the delay, and have the pio program loop with the current delay until changed.
		int stepsToAdd = 4; // Add up to 4 steps at a time
		while (!pio_sm_is_tx_fifo_full(pio, sm) && stepsToAdd > 0)
		{
			uint32_t delay = static_cast<uint32_t>(aDelay);
			pio_sm_put_blocking(pio, sm, delay);
			pio_sm_put_blocking(pio, sm, delay);

			stepsToAdd--;
		}
	}

	uint32_t PIOStepper::GetCurrentSpeed()
	{
		return myCurrentSpeed;
	}

	void PIOStepper::SetSpeed(uint32_t speed)
	{
		myTargetSpeed = speed;
	}

	uint32_t PIOStepper::GetSetSpeed()
	{
		return myTargetSpeed;
	}

	bool PIOStepper::GetDirection()
	{
		return myDirection;
	}

	bool PIOStepper::GetTargetDirection()
	{
		return myTargetDirection;
	}

	uint32_t PIOStepper::GetTargetSpeed()
	{
		return myTargetSpeed;
	}

	bool PIOStepper::IsEnabled()
	{
		return enabled;
	}

	void PIOStepper::Enable()
	{
		gpio_put(mySettingsManager->Get()->driver.driverEnPin, mySettingsManager->Get()->driver.driverEnableValue);
		enabled = true;
		DirectionChangedWait();
	}

	void PIOStepper::Disable()
	{
		gpio_put(mySettingsManager->Get()->driver.driverEnPin, mySettingsManager->Get()->driver.driverDisableValue);
		enabled = false;
		DirectionChangedWait();
	}

	void PIOStepper::Update()
	{

		if (!IsEnabled())
		{
			vTaskDelay(MS_TO_TICKS(400));
			return;
		}

		uint32_t delay = 0;
		uint32_t speed = myCurrentSpeed;

		if (myCurrentSpeed == 0)
		{
			speed = mySettingsManager->Get()->mechanical.accelerationJerk;
		}

		delay = std::floor(1.0 / (speed / 1000000.0));

		if (myDirection != myTargetDirection)
		{
			if (myCurrentSpeed > 0)
			{
				delay = CalculateNextInterval(myStepsPerRev, myCurrentSpeed, myDecelerationMultiplier * myAcceleration);
			}
			else
			{
				gpio_put(mySettingsManager->Get()->driver.driverDirPin, myTargetDirection);
				myDirection = myTargetDirection;
				DirectionChangedWait();
				return;
			}
		}
		else
		{
			if (myCurrentSpeed < myTargetSpeed)
			{
				delay = CalculateNextInterval(myStepsPerRev, myCurrentSpeed, myAcceleration);
			}

			if (myCurrentSpeed > myTargetSpeed)
			{
				delay = CalculateNextInterval(myStepsPerRev, myCurrentSpeed, myDecelerationMultiplier * myAcceleration);
			}
		}

		WriteToStepper(delay);
	}

	void PIOStepper::PIOStepperUpdateStepperTask(void *pvParameters)
	{
		PIOStepper *stepper = (PIOStepper *)pvParameters;
		while (true)
		{
			stepper->Update();
			vTaskDelay(MS_TO_TICKS(5));
		}
	}

} // namespace PowerFeed::Drivers