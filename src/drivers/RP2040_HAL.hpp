#pragma once

#include "../MachineState.hpp"
#include <cstdint>
#include <memory>

// helper to boot up the RP2040 interrupts, poll the ADC values, etc...
class RP2040_HAL
{
public:
	RP2040_HAL(std::shared_ptr<MachineState> aMachineState) : myMachineState(aMachineState){};

	// start the task that polls the ADC values
	void Start();

private:
	void InterruptHandler(void *aArg);
	std::shared_ptr<MachineState> myMachineState;
};