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

	PIOStepper::PIOStepper(uint stepPin, uint dirPin, uint enPin, uint32_t maxSpeed, uint32_t acceleration, uint32_t decelerationMultiplier, PIO pio, uint sm, uint16_t stepsPerRev)
		: stepPin(stepPin), dirPin(dirPin), enPin(enPin), myAcceleration(acceleration), myDecelerationMultiplier(decelerationMultiplier), pio(pio), sm(sm), myStepsPerRev(stepsPerRev) //, IStepper(maxSpeed, acceleration)
	{
		gpio_init(dirPin);
		gpio_set_dir(dirPin, GPIO_OUT);
		gpio_init(enPin);
		gpio_set_dir(enPin, GPIO_OUT);
		uint offset = pio_add_program(pio, (const pio_program_t *)&simplestepper_program);
		pio_sm_config c = simplestepper_program_get_default_config(offset);
		sm_config_set_sideset_pins(&c, stepPin);				   // Configure the step pin for side-set operations
		sm_config_set_set_pins(&c, stepPin, 1);					   // Configure the step pin for set operations
		pio_gpio_init(pio, stepPin);							   // Initialize the GPIO pin
		pio_sm_set_consecutive_pindirs(pio, sm, stepPin, 1, true); // Set the GPIO direction to output

		// Calculate the clock divider for 200 steps per second
		float clockDiv = 125;				// 200 steps per second, each step has 2 phases (high and low)
		sm_config_set_clkdiv(&c, clockDiv); // Set the clock divider

		pio_sm_init(pio, sm, offset, &c);  // Initialize the state machine
		pio_sm_set_enabled(pio, sm, true); // Enable the state machine
	}

	void PIOStepper::Start()
	{
		Disable();

		xTaskCreate(PIOStepperUpdateStepperTask, "Stepper Task", 2048, this, 9, NULL);
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
		vTaskDelay(MS_TO_TICKS(STEPPER_DIRECTION_CHANGE_DELAY_MS));
	}

	void PIOStepper::WriteToStepper(uint32_t aDelay)
	{
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
		gpio_put(enPin, enableValue);
		enabled = true;
		DirectionChangedWait();
	}

	void PIOStepper::Disable()
	{
		gpio_put(enPin, disableValue);
		enabled = false;
		DirectionChangedWait();
	}

	void PIOStepper::Update()
	{

		if (!IsEnabled())
		{
			vTaskDelay(MS_TO_TICKS(STEPPER_DIRECTION_CHANGE_DELAY_MS));
			return;
		}

		uint32_t delay = std::floor(1.0 / ((myCurrentSpeed ? myCurrentSpeed : ACCELERATION_JERK) / 1000000.0));

		if (myDirection != myTargetDirection)
		{
			if (myCurrentSpeed > 0)
			{
				delay = CalculateNextInterval(myStepsPerRev, myCurrentSpeed, myDecelerationMultiplier * myAcceleration);
			}
			else
			{
				gpio_put(dirPin, myTargetDirection);
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
		}
	}

} // namespace PowerFeed::Drivers