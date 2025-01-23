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
		RP2040_HAL(std::shared_ptr<MachineState> aMachineState);

		// start the task that polls the ADC values
		void Start();

	private:
		static void InterruptHandler(uint gpio, uint32_t events);
		std::shared_ptr<MachineState> myMachineState;

		bool GetLeftSwitch();
		bool GetRightSwitch();
		bool GetRapidSwitch();
		uint32_t GetNormalSpeedEncoderValue();
		uint32_t GetRapidValue();
		uint32_t GetAccelerationValue();
	};

} // namespace PicoMill::Drivers