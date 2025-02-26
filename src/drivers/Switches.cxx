#include "Switches.hxx"
#include "Helpers.hxx"
#include "MachineState.hxx"
#include "Settings.hxx"
#include "StepperState.hxx"
#include "config.h"
#include "drivers/stepper/PicoStepper.hxx"
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
	template <typename DerivedStepper>
	std::shared_ptr<Switches<DerivedStepper>> Switches<DerivedStepper>::myInstance;

	template <typename DerivedStepper>
	Switches<DerivedStepper>::Switches(std::shared_ptr<SettingsManager> aSettings, std::shared_ptr<Machine<DerivedStepper>> aMachineState) : mySettingsManager(aSettings), myMachine(aMachineState)
	{
		PIN_STATES[0] = {mySettingsManager->Get()->controls.leftPin, DeviceState::LEFT_HIGH, DeviceState::LEFT_LOW};
		PIN_STATES[1] = {mySettingsManager->Get()->controls.rightPin, DeviceState::RIGHT_HIGH, DeviceState::RIGHT_LOW};
		PIN_STATES[2] = {mySettingsManager->Get()->controls.rapidPin, DeviceState::RAPID_HIGH, DeviceState::RAPID_LOW};
		Switches::myInstance = std::shared_ptr<Switches>(this);
		myGPIOEventQueue = xQueueCreate(10, sizeof(uint32_t));
		Settings::Controls controls = mySettingsManager->Get()->controls;
		if (controls.encoderBPin != controls.encoderAPin + 1)
		{
			panic("Switches: Encoder pins must be adjacent");
		}
		// Configure GPIO pins as inputs with pull-ups
		gpio_init(controls.leftPin);
		gpio_init(controls.rightPin);
		gpio_init(controls.rapidPin);
		gpio_init(controls.encoderAPin);
		gpio_init(controls.encoderBPin);
		gpio_init(controls.encoderButtonPin);
		gpio_set_dir(controls.leftPin, GPIO_IN);
		gpio_set_dir(controls.rightPin, GPIO_IN);
		gpio_set_dir(controls.rapidPin, GPIO_IN);
		gpio_set_dir(controls.encoderAPin, GPIO_IN);
		gpio_set_dir(controls.encoderBPin, GPIO_IN);
		gpio_set_dir(controls.encoderButtonPin, GPIO_IN);
		gpio_pull_up(controls.leftPin);
		gpio_pull_up(controls.rightPin);
		gpio_pull_up(controls.rapidPin);
		gpio_pull_up(controls.encoderAPin);
		gpio_pull_up(controls.encoderBPin);
		gpio_pull_up(controls.encoderButtonPin);
		// we don't really need to keep the offset, as this program must be loaded
		// at offset 0
		pio_add_program(myEncPio, &quadrature_encoder_program);
		quadrature_encoder_program_init(myEncPio, myEncSm, controls.encoderAPin, 13300);
	}

	template <typename DerivedStepper>
	void Switches<DerivedStepper>::Start()
	{
		assert(mySettingsManager != nullptr);
		Settings::Controls controls = mySettingsManager->Get()->controls;
		// Start the encoder update task
		xTaskCreate(EncoderUpdateTask, "Encoder Task", 2048, NULL, 10, NULL);
		// Start the switch update task
		xTaskCreate(SwitchUpdateTask, "Switch Task", 2048, &myInstance, 10, NULL);
		// Set up GPIO interrupts
		gpio_set_irq_enabled_with_callback(controls.leftPin, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &SwitchInterruptHandler);
		gpio_set_irq_enabled_with_callback(controls.rightPin, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &SwitchInterruptHandler);
		gpio_set_irq_enabled_with_callback(controls.rapidPin, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &SwitchInterruptHandler);
		gpio_set_irq_enabled_with_callback(controls.encoderButtonPin, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &SwitchInterruptHandler);
		// gpio_set_irq_enabled_with_callback(ACCELERATION_PIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &SwitchInterruptHandler);
	}

	template <typename DerivedStepper>
	std::shared_ptr<Switches<DerivedStepper>> Switches<DerivedStepper>::GetInstance()
	{
		if (myInstance == nullptr)
		{
			throw std::runtime_error("Switches instance not initialized");
		}
		return myInstance;
	}

	template <typename DerivedStepper>
	void Switches<DerivedStepper>::SwitchUpdateTask(void *anInstance)
	{
		auto instance = *static_cast<std::shared_ptr<Switches<DerivedStepper>> *>(anInstance);
		Settings::Controls controls = instance->mySettingsManager->Get()->controls;
		while (true)
		{
			// blocks until it gets a gpio event
			uint32_t gpio;
			xQueueReceive(instance->myGPIOEventQueue, &gpio, portMAX_DELAY);
			auto currentTime = time_us_32();
			for (size_t i = 0; i < sizeof(instance->PIN_STATES) / sizeof(instance->PIN_STATES[0]); i++)
			{
				if (gpio == instance->PIN_STATES[i].pin)
				{
					// Check debounce window
					if (currentTime - instance->myLastPinTimes[i] < controls.debounceDelayUs)
					{
						return;
					}
					bool pinHigh = !gpio_get(gpio); // switches are active-low
					auto stateChange = std::make_shared<StateChange>(
						pinHigh ? instance->PIN_STATES[i].highState : instance->PIN_STATES[i].lowState);
					instance->myMachine->OnValueChange(stateChange);
					instance->myLastPinTimes[i] = currentTime;
					break;
				}
			}
			if (gpio == controls.encoderButtonPin)
			{
				if (currentTime - instance->myEncoderButtonLastTime < controls.debounceDelayUs)
				{
					return;
				}
				bool pinHigh = !gpio_get(gpio);
				if (!pinHigh && (currentTime - instance->myEncoderButtonLastTime > controls.unitsSwitchDelayMs * 1000))
				{
					BaseType_t woken = false;
					auto stateChange = std::make_shared<StateChange>(DeviceState::UNITS_TOGGLE);
					instance->myMachine->OnValueChange(stateChange);
				}
				instance->myEncoderButtonLastTime = currentTime;
			}
			taskYIELD();
		}
	}

	template <typename DerivedStepper>
	void Switches<DerivedStepper>::SwitchInterruptHandler(uint gpio, uint32_t events)
	{
		BaseType_t higherPriorityTaskWoken = pdFALSE;
		auto instance = GetInstance();
		xQueueSendFromISR(instance->myGPIOEventQueue, &gpio, &higherPriorityTaskWoken);
		portYIELD_FROM_ISR(higherPriorityTaskWoken);
	}

	template <typename DerivedStepper>
	void Switches<DerivedStepper>::EncoderUpdateTask(void *param)
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
			vTaskDelay(MS_TO_TICKS(100));
		}
	}

	// Explicit instantiations for the template class
	template class Switches<PowerFeed::Drivers::PicoStepper>;

} // namespace PowerFeed::Drivers