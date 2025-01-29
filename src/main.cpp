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
#include "Display.hpp"
#include "Stepper.hpp"
#include "StepperState.hpp"
#include "config.hpp"
#include "drivers/PIOStepper.hpp"
#include "drivers/PicoSSD1306Display.hpp"

std::shared_ptr<PicoMill::IStepper> stepper;
std::shared_ptr<PicoMill::Time> iTime;

std::shared_ptr<PicoMill::StepperState> stepperState;

std::shared_ptr<PicoMill::Machine> machineState;

std::unique_ptr<PicoMill::Drivers::RP2040_HAL> hal;

std::shared_ptr<PicoMill::Display> display;

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
	display = std::make_shared<PicoMill::Drivers::PicoSSD1306Display>();
	stepper = std::make_shared<PicoMill::Drivers::PIOStepper>(PicoMill::Drivers::PIOStepper(stepPinStepper, dirPinStepper, enablePinStepper, maxStepsPerSecond, ACCELERATION, DECELERATION_MULTIPLIER, pio0, 0, stepsPerMotorRev));
	iTime = std::make_shared<PicoMill::Time>();
	stepperState = std::make_shared<PicoMill::StepperState>(stepper, iTime);
	machineState = std::make_shared<PicoMill::Machine>(display, stepperState);

	// todo: load saved units and speed from eeprom

	hal = std::make_unique<PicoMill::Drivers::RP2040_HAL>(machineState);

	display->Clear();
	display->DrawSpeed(0);

	createStepperTask();
	vTaskStartScheduler();
	return 0;
}