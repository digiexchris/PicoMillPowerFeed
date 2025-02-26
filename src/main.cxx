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

std::shared_ptr<SettingsManager> settingsManager;
std::shared_ptr<DefaultStepperType> stepper;
std::shared_ptr<PowerFeed::Time> iTime;
std::shared_ptr<StepperState<DefaultStepperType>> stepperState;

std::shared_ptr<Machine<DefaultStepperType>> machineState;

std::unique_ptr<Drivers::Switches<DefaultStepperType>> switches;

std::shared_ptr<Display> display;

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
		panic("Main: Failed to create stepper task\n");
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
	printf("Starting PowerFeed\n");

	settingsManager = std::make_shared<SettingsManager>();
	auto settings = settingsManager->Load();

	if (settings == nullptr)
	{
		panic("Main: Failed to load settings\n");
		return 1;
	}

	if (settings->display.useSsd1306)
	{
		display = std::make_shared<SSD1306Display>(settingsManager);
	}
	else
	{
		display = std::make_shared<ConsoleDisplay>(settingsManager);
	}

	display->DrawStart();
	display->WriteBuffer();
	sleep_ms(500);

	stepper = std::make_shared<DefaultStepperType>(settingsManager, pio0, 0);
	iTime = std::make_shared<Time>();
	stepperState = std::make_shared<StepperState<DefaultStepperType>>(settingsManager, stepper, iTime);
	machineState = std::make_shared<Machine<DefaultStepperType>>(settingsManager, display, stepperState);

	// todo: load saved units and speed from eeprom

	switches = std::make_unique<Switches<DefaultStepperType>>(settingsManager, machineState);

	switches->Start();

	printf("Started Subsystems\n");

	createStepperTask();

	printf("Stepper Task Started\n");

	printf("Starting FreeRTOS\n");
	vTaskStartScheduler();

	printf("It should never get here");
	return 0;
}