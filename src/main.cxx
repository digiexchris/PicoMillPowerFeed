#include "Helpers.hxx"
#include "Settings.hxx"
#include "UI.hxx"
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
// #include "StepperState.hxx"
#include "Assert.hxx"
#include "config.h"
#include "drivers/display/ConsoleDisplay.hxx"
#include "drivers/display/SSD1306Display.hxx"
#include "drivers/stepper/PicoStepper.hxx"

using namespace PowerFeed;
using namespace PowerFeed::Drivers;

SettingsManager *settingsManager;
PicoStepper *stepper;
PowerFeed::Time *iTime;
UI<PicoStepper> *uiState;
Drivers::Switches<PicoStepper> *switches;
Display *display;

// Forward declaration of the HardFault_Handler
extern "C" void isr_hardfault(void);

extern "C" void PrintStackTrace(uint32_t *stackPointer);

void PrintStackTrace(uint32_t *stackPointer)
{
	BREAKPOINT();
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

extern "C" void __attribute__((naked)) isr_hardfault(void)
{
	BREAKPOINT();
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

// __asm volatile(
// 	"movs r0, #4 \n"
// 	"mov r1, lr \n"
// 	"tst r0, r1 \n"
// 	"beq _MSP \n"
// 	"mrs r0, psp \n"
// 	"b _HALT \n"
// 	"_MSP: \n"
// 	"mrs r0, msp \n"
// 	"_HALT: \n"
// 	"ldr r1, [r0, #20] \n"  // Get PC value from stack
// 	"ldr r2, =PrintStackTrace \n"
// 	"bx r2 \n"
// 	".align 4"
// );

using namespace PowerFeed;
using namespace PowerFeed::Drivers;

int main()
{
	// board_init(); //todo TINYUSB

	set_sys_clock_hz(125000000, true);
	stdio_init_all();

	// sleep_ms(500);
	printf("Starting PowerFeed\n");
	iTime = new Time();
	settingsManager = new SettingsManager();
	auto settings = settingsManager->Get();

	if (settings == nullptr)
	{
		Panic("Main: Failed to load settings\n");
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
	// sleep_ms(500);

	stepper = new PicoStepper(settingsManager, iTime, pio0, 0);

	// stepperState = new StepperState<DefaultStepperType>(
	// 	settingsManager,
	// 	stepper);

	uiState = new UI<PicoStepper>(
		settingsManager,
		display,
		stepper,
		10,
		settingsManager->Get()->mechanical.maxDriverStepsPerSecond);

	// todo: load saved units and speed from eeprom

	switches = new Switches<PicoStepper>(settingsManager, uiState);

	printf("Started Subsystems\n");

	printf("Stepper Task Started\n");

	printf("Starting FreeRTOS\n");
	vTaskStartScheduler();

	printf("It should never get here");
	return 0;
}