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
#include "drivers/PicoSSD1306Display.hxx"
#include <stdio.h>

// std::shared_ptr<PicoMill::IStepper> stepper;
// std::shared_ptr<PicoMill::Time> iTime;

// std::shared_ptr<PicoMill::StepperState> stepperState;

// std::shared_ptr<PicoMill::Machine> machineState;

// std::unique_ptr<PicoMill::Drivers::RP2040_HAL> hal;

static PicoMill::Drivers::PicoSSD1306Display display = PicoMill::Drivers::PicoSSD1306Display();

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
	picomill_main(int argc, FAR char *argv[])
	{
		syslog_initialize();
		usleep(MS_TO_US(2000));
		printf("Starting PicoMill\n");
		display.DrawStart();
		display.WriteBuffer();
		usleep(MS_TO_US(500));
		// stepper = std::make_shared<PicoMill::Drivers::PIOStepper>(PicoMill::Drivers::PIOStepper(stepPinStepper, dirPinStepper, enablePinStepper, maxStepsPerSecond, ACCELERATION, DECELERATION_MULTIPLIER, pio0, 0, stepsPerMotorRev));
		// iTime = std::make_shared<PicoMill::Time>();
		// stepperState = std::make_shared<PicoMill::StepperState>(stepper, iTime);
		// machineState = std::make_shared<PicoMill::Machine>(display, stepperState);

		// // todo: load saved units and speed from eeprom

		// hal = std::make_unique<PicoMill::Drivers::RP2040_HAL>(machineState);

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