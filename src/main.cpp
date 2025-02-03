#include "MachineState.hpp"
#include "drivers/RP2040_HAL.hpp"
#include <iostream>
#include <memory>
#include <pico/stdio.h>
#include <pico/time.h>

extern "C"
{
#include "FreeRTOS.h"
#include "task.h"
#include <timers.h>
}

#include "Common.hpp"
#include "Display.hpp"
#include "Stepper.hpp"
#include "StepperState.hpp"
#include "config.hpp"
#include "drivers/PIOStepper.hpp"
#include "drivers/PicoSSD1306Display.hpp"

std::shared_ptr<PicoMill::IStepper> stepper;
std::shared_ptr<PicoMill::Time> iTime;

std::shared_ptr<PicoMill::StepperState> stepperState;

std::shared_ptr<PicoMill::Machine> machineState;

std::unique_ptr<PicoMill::Drivers::RP2040_HAL> hal;

std::shared_ptr<PicoMill::Display> display;

void stepperUpdateTask(void *pvParameters)
{
	while (true)
	{
		stepperState->Run();
		taskYIELD();
	}
}

void createStepperTask()
{
	xTaskCreate(stepperUpdateTask, "Stepper Task", 2048, NULL, 10, NULL);
}

// Forward declaration of the HardFault_Handler
extern "C" void isr_hardfault(void);

extern "C" void PrintStackTrace(uint32_t *stackPointer);

void PrintStackTrace(uint32_t *stackPointer)
{
	printf("Hard Fault detected!\n");
	printf("R0  = %08x\n", stackPointer[0]);
	printf("R1  = %08x\n", stackPointer[1]);
	printf("R2  = %08x\n", stackPointer[2]);
	printf("R3  = %08x\n", stackPointer[3]);
	printf("R12 = %08x\n", stackPointer[4]);
	printf("LR  = %08x\n", stackPointer[5]);
	printf("PC  = %08x\n", stackPointer[6]);
	printf("PSR = %08x\n", stackPointer[7]);

	while (true)
	{
		tight_loop_contents();
	}
}

extern "C" void isr_hardfault(void)
{
	__breakpoint();
	__asm volatile(
		"MOVS R0, #4 \n"
		"MOV R1, LR \n"
		"TST R0, R1 \n"
		"BEQ _MSP \n"
		"MRS R0, PSP \n"
		"B PrintStackTrace \n"
		"_MSP: \n"
		"MRS R0, MSP \n"
		"B PrintStackTrace \n");
}

int main()
{
	stdio_init_all();
	sleep_ms(2000);
	printf("Starting PicoMill\n");

	// Start scheduler AFTER creating any timers
	if (xTimerCreateTimerTask() != pdPASS)
	{
		// Handle timer service task creation failure
		panic("Could not start timer service\n");
	}

	display = std::make_shared<PicoMill::Drivers::PicoSSD1306Display>();
	display->DrawStart();
	display->WriteBuffer();
	sleep_ms(500);
	stepper = std::make_shared<PicoMill::Drivers::PIOStepper>(PicoMill::Drivers::PIOStepper(stepPinStepper, dirPinStepper, enablePinStepper, maxStepsPerSecond, ACCELERATION, DECELERATION_MULTIPLIER, pio0, 0, stepsPerMotorRev));
	iTime = std::make_shared<PicoMill::Time>();
	stepperState = std::make_shared<PicoMill::StepperState>(stepper, iTime);
	machineState = std::make_shared<PicoMill::Machine>(display, stepperState);

	// todo: load saved units and speed from eeprom

	hal = std::make_unique<PicoMill::Drivers::RP2040_HAL>(machineState);

	hal->Start();

	printf("Started Subsystems\n");

	createStepperTask();

	printf("Stepper Task Started\n");

	printf("Starting FreeRTOS\n");
	vTaskStartScheduler();

	printf("It should never get here");
	return 0;
}