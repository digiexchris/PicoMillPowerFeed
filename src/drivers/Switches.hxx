#pragma once

#include "../MachineState.hxx"
#include "../drivers/stepper/PicoStepper.hxx"
#include "Settings.hxx"
#include "config.h"
#include <FreeRTOS.h>
#include <array>
#include <cstdint>
#include <hardware/gpio.h>
#include <hardware/pio.h>
#include <memory>
#include <pico/mutex.h>
#include <queue.h>

namespace PowerFeed::Drivers
{

	struct PinStateMapping
	{
		uint pin;
		DeviceState highState;
		DeviceState lowState;
	};

	template <typename DerivedStepper = PowerFeed::Drivers::PicoStepper>
	class Switches
	{
	public:
		Switches(std::shared_ptr<SettingsManager> aSettings, std::shared_ptr<Machine<DerivedStepper>> aMachineState);

		static std::shared_ptr<Switches<DerivedStepper>> GetInstance();

		void Start();

	private:
		static std::shared_ptr<Switches<DerivedStepper>> myInstance;
		std::shared_ptr<Machine<DerivedStepper>> myMachine;
		std::shared_ptr<SettingsManager> mySettingsManager;

		static void SwitchInterruptHandler(uint gpio, uint32_t events);
		static void EncoderUpdateTask(void *instance);
		static void SwitchUpdateTask(void *instance);
		uint32_t myEncNewValue = 0;
		uint32_t myEncOldValue = 0;
		uint8_t myLastEncState = 0;
		const PIO myEncPio = pio1;
		const uint myEncSm = 0;
		QueueHandle_t myGPIOEventQueue;

		PinStateMapping PIN_STATES[3];

		std::array<uint32_t, sizeof(PIN_STATES) / sizeof(PIN_STATES[0])> myLastPinTimes = {0};

		uint32_t myEncoderButtonLastTime = 0;
		bool myEncoderButtonLastState = false;
	};

} // namespace PowerFeed::Drivers