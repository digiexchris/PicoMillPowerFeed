#include <iostream>
#include <memory>

#include "Common.hpp"
#include "PIOStepper.hpp"
#include "Stepper.hpp"
#include "StepperState.hpp"
#include "config.hpp"

std::shared_ptr<IStepper> stepper = std::make_shared<PIOStepper>(PIOStepper(stepPinStepper, dirPinStepper, enablePinStepper, 0, ACCELERATION, DECELERATION, pio0, 0, stepsPerMotorRev));

int main()
{
	std::shared_ptr<Time> time = std::make_shared<Time>();
	StepperState state(stepper, time);
	std::cout << "Hello, World!" << std::endl;
	return 0;
}