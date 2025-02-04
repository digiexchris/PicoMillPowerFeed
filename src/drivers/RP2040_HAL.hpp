#pragma once

#include "../MachineState.hpp"
#include "config.hpp"
// #include <FreeRTOS.h>
#include <stdint.h>
// #include <hardware/gpio.h>
// #include <hardware/pio.h>
// #include <memory.h>
// #include <pico/mutex.h>
// #include <queue.h>
#include <array.h>

namespace PicoMill::Drivers
{

	struct PinStateMapping
	{
		uint pin;
		DeviceState highState;
		DeviceState lowState;
	};

	class RP2040_HAL
	{
	public:
		RP2040_HAL(std::shared_ptr<Machine> aMachineState);

		// start the task that polls the ADC values
		static std::shared_ptr<RP2040_HAL> GetInstance();

		void Start();

	private:
		static std::shared_ptr<RP2040_HAL> myInstance;
		std::shared_ptr<Machine> myMachine;
		static void SwitchInterruptHandler(uint gpio, uint32_t events);
		static void SwitchInterruptHandlerImpl(void *instance, uint32_t gpio);
		static void EncoderUpdateTask(void *instance);
		uint32_t myEncNewValue = 0;
		uint32_t myEncOldValue = 0;
		uint8_t myLastEncState = 0;
		const PIO myEncPio = pio1;
		const uint myEncSm = 0;

		static constexpr PinStateMapping PIN_STATES[] = {
			{LEFTPIN, DeviceState::LEFT_HIGH, DeviceState::LEFT_LOW},
			{RIGHTPIN, DeviceState::RIGHT_HIGH, DeviceState::RIGHT_LOW},
			{RAPIDPIN, DeviceState::RAPID_HIGH, DeviceState::RAPID_LOW},
			{ACCELERATION_PIN, DeviceState::ACCELERATION_HIGH, DeviceState::ACCELERATION_LOW}};

		std::array<uint32_t, sizeof(PIN_STATES) / sizeof(PIN_STATES[0])> myLastPinTimes = {0};

		uint32_t myEncoderButtonLastTime = 0;
		bool myEncoderButtonLastState = false;
	};

} // namespace PicoMill::Drivers