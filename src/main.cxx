#include "Helpers.hxx"
#include "MachineState.hxx"
#include "Settings.hxx"
// #include "bsp/board_api.h" //todo TINYUSB
#include "drivers/Switches.hxx"
#include <FreeRTOS.h>
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
#include "drivers/display/ConsoleDisplay.hxx"
#include "drivers/display/SSD1306Display.hxx"
#include "drivers/stepper/PicoStepper.hxx"

using namespace PowerFeed;
using namespace PowerFeed::Drivers;

SettingsManager *settingsManager;
DefaultStepperType *stepper;
PowerFeed::Time *iTime;
StepperState<DefaultStepperType> *stepperState;
Machine<DefaultStepperType> *machineState;
Drivers::Switches<DefaultStepperType> *switches;
Display *display;

void stepperUpdateTask(void *pvParameters)
{
	while (true)
	{
		stepperState->Run();
		vTaskDelay(MS_TO_TICKS(20));
		taskYIELD();
	}
}

void createStepperTask()
{

	auto result = xTaskCreate(stepperUpdateTask, "Stepper Task", 2048, NULL, 13, NULL);

	if (result != pdPASS)
	{
		BreakPanic("Main: Failed to create stepper task\n");
	}
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

	fflush(stdout);

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

using namespace PowerFeed;
using namespace PowerFeed::Drivers;

int main()
{
	// board_init(); //todo TINYUSB

	set_sys_clock_hz(133000000, true);
	stdio_init_all();

	sleep_ms(500);
	printf("Starting PowerFeed\n");
	iTime = new Time();
	settingsManager = new SettingsManager();
	auto settings = settingsManager->Get();

	if (settings == nullptr)
	{
		BreakPanic("Main: Failed to load settings\n");
		return 1;
	}

	if (settings->display.useSsd1306)
	{
		display = new SSD1306Display(settingsManager);
	}
	else
	{
		display = new ConsoleDisplay(settingsManager);
	}

	display->DrawStart();
	display->WriteBuffer();
	sleep_ms(500);

	stepper = new DefaultStepperType(settingsManager, iTime, pio0, 0);

	stepperState = new StepperState<DefaultStepperType>(
		settingsManager,
		stepper);

	machineState = new Machine<DefaultStepperType>(
		settingsManager,
		display,
		stepperState,
		10,
		settingsManager->Get()->mechanical.maxDriverStepsPerSecond);

	// todo: load saved units and speed from eeprom

	switches = new Switches<DefaultStepperType>(settingsManager, machineState);

	switches->Start();

	printf("Started Subsystems\n");

	createStepperTask();

	printf("Stepper Task Started\n");

	printf("Starting FreeRTOS\n");
	vTaskStartScheduler();

	printf("It should never get here");
	return 0;
}