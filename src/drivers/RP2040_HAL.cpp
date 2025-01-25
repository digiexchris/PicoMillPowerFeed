#include "RP2040_HAL.hpp"

#include "Helpers.hpp"
#include "StepperState.hpp"
#include "config.hpp"
#include <FreeRTOS.h>
#include <hardware/gpio.h>
#include <hardware/irq.h>
#include <memory>
#include <task.h>

#include "MachineState.hpp"

namespace PicoMill::Drivers
{

	RP2040_HAL::RP2040_HAL(std::shared_ptr<Machine> aMachineState)
	{
		myMachine = aMachineState;

		// Configure GPIO pins as inputs with pull-ups
		gpio_init(LEFTPIN);
		gpio_init(RIGHTPIN);
		gpio_init(RAPIDPIN);
		gpio_init(ENCODER_A_PIN);
		gpio_init(ENCODER_B_PIN);
		gpio_init(ENCODER_BUTTON_PIN);

		gpio_set_dir(LEFTPIN, GPIO_IN);
		gpio_set_dir(RIGHTPIN, GPIO_IN);
		gpio_set_dir(RAPIDPIN, GPIO_IN);
		gpio_set_dir(ENCODER_A_PIN, GPIO_IN);
		gpio_set_dir(ENCODER_B_PIN, GPIO_IN);
		gpio_set_dir(ENCODER_BUTTON_PIN, GPIO_IN);

		gpio_pull_up(LEFTPIN);
		gpio_pull_up(RIGHTPIN);
		gpio_pull_up(RAPIDPIN);
		gpio_pull_up(ENCODER_A_PIN);
		gpio_pull_up(ENCODER_B_PIN);
		gpio_pull_up(ENCODER_BUTTON_PIN);

		// Set up GPIO interrupts
		gpio_set_irq_enabled_with_callback(LEFTPIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &InterruptHandler);
		gpio_set_irq_enabled_with_callback(RIGHTPIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &InterruptHandler);
		gpio_set_irq_enabled_with_callback(RAPIDPIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &InterruptHandler);
		gpio_set_irq_enabled_with_callback(ENCODER_A_PIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &InterruptHandler);
		gpio_set_irq_enabled_with_callback(ENCODER_B_PIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &InterruptHandler);
		gpio_set_irq_enabled_with_callback(ENCODER_BUTTON_PIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &InterruptHandler);
	}

	void RP2040_HAL::Start()
	{
	}

	void RP2040_HAL::Poll(void *pvParameters)
	{
		RP2040_HAL *hal = static_cast<RP2040_HAL *>(pvParameters);

		auto machine = hal->myMachine;

		while (true)
		{
			auto rapidSpeed = hal->GetRapidValue();
			if (!IsWithinRange<uint32_t>(rapidSpeed, hal->myRapidValue, RAPID_SPEED_PIN_DEADZONE))
			{
				machine->OnValueChange(std::make_shared<PicoMill::UInt32StateChange>(PicoMill::DeviceState::RAPID_SPEED_CHANGE, rapidSpeed));
				hal->myRapidValue = rapidSpeed;
			}

			auto acceleration = hal->GetAccelerationValue();
			if (!IsWithinRange<uint32_t>(acceleration, hal->myAccelerationValue, ACCELERATION_PIN_DEADZONE))
			{
				machine->OnValueChange(std::make_shared<PicoMill::UInt32StateChange>(PicoMill::DeviceState::NORMAL_SPEED_CHANGE, acceleration));
				hal->myAccelerationValue = acceleration;
			}

			vTaskDelay(MS_TO_TICKS(200));
		}
	}

	bool RP2040_HAL::GetLeftSwitch()
	{
		return false;
	}

	bool RP2040_HAL::GetRightSwitch()
	{
		return false;
	}

	bool RP2040_HAL::GetRapidSwitch()
	{
		return false;
	}

	uint32_t RP2040_HAL::GetNormalSpeedEncoderValue()
	{
		return 0;
	}

	uint32_t RP2040_HAL::GetRapidValue()
	{
		return 0;
	}

	uint32_t RP2040_HAL::GetAccelerationValue()
	{
		return 0;
	}

	void RP2040_HAL::InterruptHandler(uint gpio, uint32_t events)
	{
	}

} // namespace PicoMill::Drivers