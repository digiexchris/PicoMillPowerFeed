#include "MachineState.hpp"
#include "drivers/RP2040_HAL.hpp"
#include <iostream>
#include <memory>

extern "C"
{
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
}

#include "Common.hpp"
#include "PIOStepper.hpp"
#include "Stepper.hpp"
#include "StepperState.hpp"
#include "config.hpp"

std::shared_ptr<IStepper> stepper;
std::shared_ptr<Time> time;

std::unique_ptr<StepperState> stepperSTate;

std::shared_ptr<MachineState> machineState;

std::unqqique_ptr<RP2040_HAL> hal;

void stepperUpdateTask(void *pvParameters)
{
	while (true)
	{
		stepperSTate->Run();
		taskYIELD();
	}
}

void createStepperTask()
{
	xTaskCreatePinnedToCore(stepperUpdateTask, "Stepper Task", 2048, NULL, 1, NULL, 1);
}

int main()
{
	stepper = std::make_shared<PIOStepper>(PIOStepper(stepPinStepper, dirPinStepper, enablePinStepper, 0, ACCELERATION, DECELERATION, pio0, 0, stepsPerMotorRev));
	time = std::make_shared<Time>();
	stepperSTate = std::make_unique<StepperState>(stepper, time);

	createStepperTask();

	machineState = std::make_shared<MachineState>();

	hal = std::make_unique<RP2040_HAL>(machineState);

	hal.Start(); //start the task to poll the ADC

	vTaskStartScheduler();
	return 0;
}