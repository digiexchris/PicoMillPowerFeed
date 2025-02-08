#include "MachineState.hxx"
// #include "drivers/RP2040_HAL.hxx"
// #include <memory.h>
#include <nuttx/config.h>
#include <unistd.h>

#include "Common.hxx"
#include "Display.hxx"
#include "Stepper.hxx"
#include "StepperState.hxx"
#include "config.hxx"
// #include "drivers/PIOSte7pper.hpp"
#include "Helpers.hxx"
#include "drivers/ConsoleDisplay.hxx"
#include <stdio.h>

// std::shared_ptr<PowerFeed::IStepper> stepper;
// std::shared_ptr<PowerFeed::Time> iTime;

// std::shared_ptr<PowerFeed::StepperState> stepperState;

// std::shared_ptr<PowerFeed::Machine> machineState;

// std::unique_ptr<PowerFeed::Drivers::RP2040_HAL> hal;

static PowerFeed::Drivers::ConsoleDisplay display = PowerFeed::Drivers::ConsoleDisplay();

// void stepperUpdateTask(void *pvParameters)
// {
// 	while (true)
// 	{
// 		stepperState->Run();
// 		taskYIELD();
// 	}
// }

// void createStepperTask()
// {
// 	xTaskCreate(stepperUpdateTask, "Stepper Task", 2048, NULL, 10, NULL);
// }
extern "C"
{
	int
	PowerFeed_main(int argc, FAR char *argv[])
	{
		// syslog_initialize();
		usleep(MS_TO_US(2000));
		printf("Starting PowerFeed\n");
		display.DrawStart();
		display.WriteBuffer();
		usleep(MS_TO_US(500));
		// stepper = std::make_shared<PowerFeed::Drivers::PIOStepper>(PowerFeed::Drivers::PIOStepper(stepPinStepper, dirPinStepper, enablePinStepper, maxStepsPerSecond, ACCELERATION, DECELERATION_MULTIPLIER, pio0, 0, stepsPerMotorRev));
		// iTime = std::make_shared<PowerFeed::Time>();
		// stepperState = std::make_shared<PowerFeed::StepperState>(stepper, iTime);
		// machineState = std::make_shared<PowerFeed::Machine>(display, stepperState);

		// // todo: load saved units and speed from eeprom

		// hal = std::make_unique<PowerFeed::Drivers::RP2040_HAL>(machineState);

		// hal->Start();

		// printf("Started Subsystems\n");

		// createStepperTask();

		// printf("Stepper Task Started\n");

		// printf("Starting FreeRTOS\n");
		// vTaskStartScheduler();

		// printf("It should never get here");
		while (true)
		{
			usleep(MS_TO_US(1000));
		}
		return 0;
	}
}