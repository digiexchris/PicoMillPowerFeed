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

std::shared_ptr<PicoMill::StepperState> stepperState;

std::shared_ptr<PicoMill::Machine> machineState;

std::unique_ptr<PicoMill::Drivers::RP2040_HAL> hal;

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
	xTaskCreate(stepperUpdateTask, "Stepper Task", 2048, NULL, 1, NULL);
}

int main()
{
	stepper = std::make_shared<PicoMill::Drivers::PIOStepper>(PicoMill::Drivers::PIOStepper(stepPinStepper, dirPinStepper, enablePinStepper, maxStepsPerSecond, ACCELERATION, DECELERATION, pio0, 0, stepsPerMotorRev));
	iTime = std::make_shared<PicoMill::Time>();
	stepperState = std::make_shared<PicoMill::StepperState>(stepper, iTime);

	createStepperTask();

	machineState = std::make_shared<PicoMill::Machine>(stepperState);

	hal = std::make_unique<PicoMill::Drivers::RP2040_HAL>(machineState);

	hal->Start(); // start the task to poll the ADC

	vTaskStartScheduler();
	return 0;
}