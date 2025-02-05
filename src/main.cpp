#include "MachineState.hpp"
#include "drivers/RP2040_HAL.hpp"
// #include <memory.h>
#include <nuttx/config.h>
#include <unistd.h>

#include "Common.hpp"
#include "Display.hpp"
#include "Stepper.hpp"
#include "StepperState.hpp"
#include "config.hpp"
// #include "drivers/PIOSte7pper.hpp"
#include "Helpers.hpp"
#include "drivers/PicoSSD1306Display.hpp"
// #include <stdio.h>

// std::shared_ptr<PicoMill::IStepper> stepper;
// std::shared_ptr<PicoMill::Time> iTime;

// std::shared_ptr<PicoMill::StepperState> stepperState;

// std::shared_ptr<PicoMill::Machine> machineState;

// std::unique_ptr<PicoMill::Drivers::RP2040_HAL> hal;

PicoMill::Drivers::PicoSSD1306Display display = PicoMill::Drivers::PicoSSD1306Display();

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

int main()
{
	usleep(MS_TO_US(2000));
	// printf("Starting PicoMill\n");
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
	return 0;
}