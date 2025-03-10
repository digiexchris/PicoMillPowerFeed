
#include "bsp/board_api.h" // TinyUSB

#include <FreeRTOS.h>
#include <hardware/watchdog.h>
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

#include "Assert.hxx"
#include "Common.hxx"
#include "Display.hxx"
#include "Helpers.hxx"
#include "Settings.hxx"
#include "Stepper.hxx"
#include "UI.hxx"
#include "config.h"
#include "drivers/Switches.hxx"
#include "drivers/USBMassStorage/LittleFSSettings.hxx"
#include "drivers/USBMassStorage/TinyUSBMSC.hxx"
#include "drivers/display/ConsoleDisplay.hxx"
#include "drivers/display/SSD1306Display.hxx"
#include "drivers/stepper/PicoStepper.hxx"

using namespace PowerFeed;
using namespace PowerFeed::Drivers;

// Use LittleFSSettings instead of SettingsManager
std::shared_ptr<LittleFSSettings> settingsManager;
TinyUSBMSC *usbMsc;

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

// USB Task for TinyUSB processing
static void UsbTask(void *pvParameters)
{
	while (1)
	{
		// Service USB events
		if (usbMsc != nullptr)
		{
			usbMsc->Task();
		}
		vTaskDelay(pdMS_TO_TICKS(10));
	}
}

// Config update callback - called when config.json is updated via USB
void ConfigUpdatedCallback()
{
	printf("Config updated via USB\n");
}

// USB drive ejected callback - called when the USB drive is ejected
void EjectedCallback()
{
	printf("USB Drive Ejected, restarting to load final config\n");

	// Wait a moment to ensure any pending writes are completed
	sleep_ms(100);

	// Enable the watchdog with a short timeout to trigger a reset
	watchdog_enable(1, false);
	while (1)
	{
		// Wait for watchdog reset
		tight_loop_contents();
	}
}

using namespace PowerFeed;
using namespace PowerFeed::Drivers;

int main()
{
	set_sys_clock_hz(125000000, true);
	stdio_init_all();

	printf("Starting PowerFeed\n");
	iTime = new Time();

	// Create LittleFSSettings instead of standard SettingsManager
	settingsManager = std::make_shared<LittleFSSettings>();
	auto settings = settingsManager->Get();

	if (settings == nullptr)
	{
		Panic("Main: Failed to load settings\n");
		return 1;
	}

	// Initialize TinyUSB with LittleFS integration
	usbMsc = new TinyUSBMSC(settingsManager);
	usbMsc->SetConfigUpdateCallback(ConfigUpdatedCallback);
	usbMsc->SetEjectedCallback(EjectedCallback);
	// Create USB task
	xTaskCreate(UsbTask, "USB_Task", configMINIMAL_STACK_SIZE * 4, NULL, tskIDLE_PRIORITY + 2, NULL);

	if (settings->display.useSsd1306)
	{
		display = new SSD1306Display(settingsManager.get());
	}
	else
	{
		display = new ConsoleDisplay(settingsManager.get());
	}

	display->DrawStart();
	display->WriteBuffer();
	sleep_ms(500);

	stepper = new PicoStepper(settingsManager.get(), iTime, pio0, 0);

	uiState = new UI<PicoStepper>(
		settingsManager.get(),
		display,
		stepper,
		10,
		settingsManager->Get()->mechanical.maxDriverStepsPerSecond);

	switches = new Switches<PicoStepper>(settingsManager.get(), uiState);

	printf("Started Subsystems\n");

	printf("Stepper Task Started\n");

	printf("Starting FreeRTOS\n");
	vTaskStartScheduler();

	printf("It should never get here");
	return 0;
}