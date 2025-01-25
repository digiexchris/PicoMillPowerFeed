#pragma once

#include "../MachineState.hpp"
#include <cstdint>
#include <hardware/gpio.h>
#include <memory>

namespace PicoMill::Drivers
{

	// helper to boot up the RP2040 interrupts, poll the ADC values, etc...
	class RP2040_HAL
	{
	public:
		RP2040_HAL(std::shared_ptr<Machine> aMachineState);

		// start the task that polls the ADC values
		void Start();

	private:
		static void InterruptHandler(uint gpio, uint32_t events);
		std::shared_ptr<Machine> myMachine;

		static void Poll(void *pvParameters);

		bool GetLeftSwitch();
		bool GetRightSwitch();
		bool GetRapidSwitch();
		uint32_t GetNormalSpeedEncoderValue();
		uint32_t myNormalSpeedEncoderValue = 0;
		uint32_t GetRapidValue();
		uint32_t myRapidValue = 0;
		uint32_t GetAccelerationValue();
		uint32_t myAccelerationValue = 0;
	};

} // namespace PicoMill::Drivers