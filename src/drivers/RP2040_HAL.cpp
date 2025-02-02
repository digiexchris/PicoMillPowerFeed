#include "RP2040_HAL.hpp"
#include "Encoder.hpp"
#include "Helpers.hpp"
#include "MachineState.hpp"
#include "StepperState.hpp"
#include "config.hpp"
#include "portmacro.h"
#include <FreeRTOS.h>
#include <hardware/gpio.h>
#include <hardware/irq.h>
#include <hardware/timer.h>
#include <memory>
#include <pico/types.h>
#include <queue.h>
#include <task.h>
#include <timers.h>

namespace PicoMill::Drivers
{
	std::shared_ptr<RP2040_HAL> RP2040_HAL::myInstance;

	RP2040_HAL::RP2040_HAL(std::shared_ptr<Machine> aMachineState)
	{
		myMachine = aMachineState;

		RP2040_HAL::myInstance = std::shared_ptr<RP2040_HAL>(this);

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
	}

	void RP2040_HAL::Start()
	{
		// Set up GPIO interrupts
		gpio_set_irq_enabled_with_callback(LEFTPIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &SwitchInterruptHandler);
		gpio_set_irq_enabled_with_callback(RIGHTPIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &SwitchInterruptHandler);
		gpio_set_irq_enabled_with_callback(RAPIDPIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &SwitchInterruptHandler);
		gpio_set_irq_enabled_with_callback(ENCODER_A_PIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &EncInterruptHandler);
		gpio_set_irq_enabled_with_callback(ENCODER_B_PIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &EncInterruptHandler);
		gpio_set_irq_enabled_with_callback(ENCODER_BUTTON_PIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &SwitchInterruptHandler);

		// gpio_set_irq_enabled_with_callback(ACCELERATION_PIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &SwitchInterruptHandler);
	}

	std::shared_ptr<RP2040_HAL> RP2040_HAL::GetInstance()
	{
		if (myInstance == nullptr)
		{
			throw std::runtime_error("RP2040_HAL instance not initialized");
		}
		return myInstance;
	}

	void RP2040_HAL::SwitchInterruptHandlerImpl(void *, uint32_t gpio)
	{
		auto instance = GetInstance();
		auto currentTime = time_us_32();

		for (size_t i = 0; i < sizeof(PIN_STATES) / sizeof(PIN_STATES[0]); i++)
		{
			if (gpio == PIN_STATES[i].pin)
			{

				// Check debounce window
				if (currentTime - instance->myLastPinTimes[i] < DEBOUNCE_DELAY_US)
				{
					return;
				}

				bool pinHigh = !gpio_get(gpio); // switches are active-low
				auto stateChange = std::make_shared<StateChange>(
					pinHigh ? PIN_STATES[i].highState : PIN_STATES[i].lowState);

				instance->myMachine->OnValueChange(stateChange);

				instance->myLastPinTimes[i] = currentTime;
				break;
			}
		}

		if (gpio == ENCODER_BUTTON_PIN)
		{
			if (currentTime - instance->myEncoderButtonLastTime < DEBOUNCE_DELAY_US)
			{
				return;
			}

			bool pinHigh = !gpio_get(gpio);
			if (!pinHigh && (currentTime - instance->myEncoderButtonLastTime > UNITS_SWITCH_DELAY_MS * 1000))
			{
				BaseType_t woken = false;
				auto stateChange = std::make_shared<StateChange>(DeviceState::UNITS_TOGGLE);

				instance->myMachine->OnValueChange(stateChange);
			}
			instance->myEncoderButtonLastTime = currentTime;
		}
	}

	void RP2040_HAL::SwitchInterruptHandler(uint gpio, uint32_t events)
	{
		BaseType_t higherPriorityTaskWoken = pdFALSE;
		xTimerPendFunctionCallFromISR(SwitchInterruptHandlerImpl,
									  nullptr,
									  gpio,
									  &higherPriorityTaskWoken);

		portYIELD_FROM_ISR(higherPriorityTaskWoken);
	}

	void RP2040_HAL::EncInterruptHandler(uint gpio, uint32_t events)
	{
		auto instance = GetInstance();
		bool pinState = events == GPIO_IRQ_EDGE_RISE;

		switch (gpio)
		{
		case ENCODER_A_PIN:
			instance->myLastEncA = pinState;
			break;
		case ENCODER_B_PIN:
			instance->myLastEncB = pinState;
			break;
		}

		// Gray code state machine for quadrature encoding
		static const uint8_t stateMap[] = {0x0, 0x2, 0x4, 0x6, 0x7, 0x5, 0x3, 0x1};
		uint8_t state = (instance->myLastEncA << 1) | instance->myLastEncB;

		static uint8_t lastState = 0;
		if (state != lastState)
		{
			// Clockwise sequence: 0,1,3,2
			if ((lastState == 0 && state == 1) ||
				(lastState == 1 && state == 3) ||
				(lastState == 3 && state == 2) ||
				(lastState == 2 && state == 0))
			{
				BaseType_t woken = false;
				auto stateChange = std::make_shared<Int8StateChange>(DeviceState::ENCODER_CHANGED, 1);

				instance->myMachine->OnValueChange(stateChange);
			}
			else
			{
				BaseType_t woken = false;
				auto stateChange = std::make_shared<Int8StateChange>(DeviceState::ENCODER_CHANGED, -1);

				instance->myMachine->OnValueChange(stateChange);
			}
		}
		lastState = state;
	}
} // namespace PicoMill::Drivers