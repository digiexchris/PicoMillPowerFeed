#include "MachineState.hpp"
#include "drivers/RP2040_HAL.hpp"
#include <iostream>
#include <memory>

extern "C"
{
#include "FreeRTOS.h"
#include "task.h"
}

#include "Common.hpp"
#include "Stepper.hpp"
#include "StepperState.hpp"
#include "config.hpp"
#include "drivers/PIOStepper.hpp"

std::shared_ptr<PicoMill::IStepper> stepper;
std::shared_ptr<PicoMill::Time> iTime;

std::unique_ptr<PicoMill::StepperState> stepperSTate;

std::shared_ptr<PicoMill::MachineState> machineState;

std::unique_ptr<PicoMill::Drivers::RP2040_HAL> hal;

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
	xTaskCreate(stepperUpdateTask, "Stepper Task", 2048, NULL, 1, NULL);
}

int main()
{
	stepper = std::make_shared<PicoMill::Drivers::PIOStepper>(PicoMill::Drivers::PIOStepper(stepPinStepper, dirPinStepper, enablePinStepper, 0, ACCELERATION, DECELERATION, pio0, 0, stepsPerMotorRev));
	iTime = std::make_shared<PicoMill::Time>();
	stepperSTate = std::make_unique<PicoMill::StepperState>(stepper, time);

	createStepperTask();

	machineState = std::make_shared<PicoMill::MachineState>();

	hal = std::make_unique<PicoMill::Drivers::RP2040_HAL>(machineState);

	hal.Start(); // start the task to poll the ADC

	vTaskStartScheduler();
	return 0;
}