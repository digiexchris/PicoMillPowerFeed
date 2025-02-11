#include "MachineState.hxx"
#include "drivers/Switches.hxx"
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

#include "Common.hxx"
#include "Display.hxx"
#include "Stepper.hxx"
#include "StepperState.hxx"
#include "config.h"
#include "drivers/ConsoleDisplay.hxx"
#include "drivers/PIOStepper.hxx"
#include "drivers/SSD1306Display.hxx"

std::shared_ptr<PowerFeed::IStepper> stepper;
std::shared_ptr<PowerFeed::Time> iTime;

std::shared_ptr<PowerFeed::StepperState> stepperState;

std::shared_ptr<PowerFeed::Machine> machineState;

std::unique_ptr<PowerFeed::Drivers::Switches> hal;

std::shared_ptr<PowerFeed::Display> display;

#if defined(USE_SSD1306)
using DISPLAY = PowerFeed::Drivers::SSD1306Display;
#else
using DISPLAY = PowerFeed::Drivers::ConsoleDisplay;
#endif

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
	printf("Starting PowerFeed\n");

	display = std::make_shared<DISPLAY>();
	display->DrawStart();
	display->WriteBuffer();
	sleep_ms(500);
	stepper = std::make_shared<PowerFeed::Drivers::PIOStepper>(PowerFeed::Drivers::PIOStepper(DRIVER_STEP_PIN, DRIVER_DIR_PIN, DRIVER_EN_PIN, maxStepsPerSecond, ACCELERATION, DECELERATION_MULTIPLIER, pio0, 0, STEPS_PER_MOTOR_REV));
	iTime = std::make_shared<PowerFeed::Time>();
	stepperState = std::make_shared<PowerFeed::StepperState>(stepper, iTime);
	machineState = std::make_shared<PowerFeed::Machine>(display, stepperState);

	// todo: load saved units and speed from eeprom

	hal = std::make_unique<PowerFeed::Drivers::Switches>(machineState);

	hal->Start();

	printf("Started Subsystems\n");

	createStepperTask();

	printf("Stepper Task Started\n");

	printf("Starting FreeRTOS\n");
	vTaskStartScheduler();

	printf("It should never get here");
	return 0;
}