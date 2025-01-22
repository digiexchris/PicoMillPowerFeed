#include "RP2040_HAL.hpp"

#include "hardware/irq.h"

RP2040_HAL::RP2040_HAL()
{

	// todo setup interrupts on pins, set a task to poll the ADC for the pots
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

void RP2040_HAL::InterruptHandler(void *aArg)
{
	uint32_t status = irq_get_status(IRQ_0); // Example for IRQ_0, change as needed

	if (status & (1 << YOUR_INTERRUPT_BIT))
	{
		myMachineState->OnInterrupt(aType);
	}

	// Clear the interrupt
	irq_clear(IRQ_0); // Example for IRQ_0, change as needed
}