// #include "Encoder.hpp"
#include "Switches.hxx"
#include "Helpers.hxx"
#include "MachineState.hxx"
#include "StepperState.hxx"
#include "config.h"
#include "portmacro.h"
#include "quadrature_encoder.pio.h"
#include <FreeRTOS.h>
#include <hardware/gpio.h>
#include <hardware/irq.h>
#include <hardware/pio.h>
#include <hardware/timer.h>
#include <memory>
#include <pico/types.h>
#include <queue.h>
#include <task.h>
#include <timers.h>

namespace PowerFeed::Drivers
{
	std::shared_ptr<Switches> Switches::myInstance;

	Switches::Switches(std::shared_ptr<Machine> aMachineState)
	{
		myMachine = aMachineState;

		Switches::myInstance = std::shared_ptr<Switches>(this);

		myGPIOEventQueue = xQueueCreate(1, sizeof(uint32_t));

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

		// we don't really need to keep the offset, as this program must be loaded
		// at offset 0
		pio_add_program(myEncPio, &quadrature_encoder_program);
		quadrature_encoder_program_init(myEncPio, myEncSm, ENCODER_A_PIN, 13300);
	}

	void Switches::Start()
	{
		// Start the encoder update task
		xTaskCreate(EncoderUpdateTask, "Encoder Task", 2048, NULL, 10, NULL);

		// Start the switch update task
		xTaskCreate(SwitchUpdateTask, "Switch Task", 2048, &myInstance, 10, NULL);

		// Set up GPIO interrupts
		gpio_set_irq_enabled_with_callback(LEFTPIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &SwitchInterruptHandler);
		gpio_set_irq_enabled_with_callback(RIGHTPIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &SwitchInterruptHandler);
		gpio_set_irq_enabled_with_callback(RAPIDPIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &SwitchInterruptHandler);
		gpio_set_irq_enabled_with_callback(ENCODER_BUTTON_PIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &SwitchInterruptHandler);

		// gpio_set_irq_enabled_with_callback(ACCELERATION_PIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &SwitchInterruptHandler);
	}

	std::shared_ptr<Switches> Switches::GetInstance()
	{
		if (myInstance == nullptr)
		{
			throw std::runtime_error("Switches instance not initialized");
		}
		return myInstance;
	}

	void
	Switches::SwitchUpdateTask(void *anInstance)
	{
		auto instance = *static_cast<std::shared_ptr<Switches> *>(anInstance);

		while (true)
		{
			// blocks until it gets a gpio event
			uint32_t gpio;
			xQueueReceive(instance->myGPIOEventQueue, &gpio, portMAX_DELAY);

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
	}

	void Switches::SwitchInterruptHandler(uint gpio, uint32_t events)
	{
		BaseType_t higherPriorityTaskWoken = pdFALSE;
		auto instance = GetInstance();

		xQueueSendFromISR(instance->myGPIOEventQueue, &gpio, &higherPriorityTaskWoken);

		portYIELD_FROM_ISR(higherPriorityTaskWoken);
	}

	void Switches::EncoderUpdateTask(void *param)
	{
		auto instance = GetInstance();
		while (1)
		{
			// note: thanks to two's complement arithmetic delta will always
			// be correct even when new_value wraps around MAXINT / MININT
			instance->myEncNewValue = quadrature_encoder_get_count(instance->myEncPio, instance->myEncSm);

			int32_t delta = static_cast<int32_t>(instance->myEncNewValue) - instance->myEncOldValue;
			instance->myEncOldValue = instance->myEncNewValue;

			if (delta != 0)
			{
				auto stateChange = std::make_shared<ValueChange<int16_t>>(DeviceState::ENCODER_CHANGED, delta);
				instance->myMachine->OnValueChange(stateChange);
			}
			sleep_ms(100);
		}
	}
} // namespace PowerFeed::Drivers