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

	template <typename DerivedStepper>
	class Switches
	{
	public:
		Switches(SettingsManager *aSettings, Machine<DerivedStepper> *aMachineState);
		void Start();

	private:
		// required by the ISR handler callback unfortunately
		static Switches<DerivedStepper> *myInstance;
		Machine<DerivedStepper> *myMachine;
		SettingsManager *mySettingsManager;

		static void SwitchInterruptHandler(uint gpio, uint32_t events);
		static void EncoderUpdateTask(void *instance);
		static void SwitchUpdateTask(void *instance);
		uint32_t myEncNewValue = 0;
		uint32_t myEncOldValue = 0;
		uint8_t myLastEncState = 0;
		PIO myEncPio = pio1;
		uint myEncSm = 0;

		uint myEncOffset;
		QueueHandle_t myGPIOEventQueue;

		PinStateMapping PIN_STATES[3];

		std::array<uint32_t, sizeof(PIN_STATES) / sizeof(PIN_STATES[0])> myLastPinTimes = {0};

		uint32_t myEncoderButtonLastTime = 0;
		bool myEncoderButtonLastState = false;
	};

} // namespace PowerFeed::Drivers