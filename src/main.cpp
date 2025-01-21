#include <iostream>

#include "PIOStepper.hpp"
#include "State.hpp"
#include "Stepper.hpp"
#include "config.hpp"

std::shared_ptr<IStepper> stepper = std::make_shared<PIOStepper>(PIOStepper(stepPinStepper, dirPinStepper, enablePinStepper, 0, ACCELERATION, DECELERATION, pio0, 0, stepsPerMotorRev));

int main()
{
	State state(stepper);
	std::cout << "Hello, World!" << std::endl;
	return 0;
}