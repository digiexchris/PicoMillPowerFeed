#pragma once

#include "../src/Stepper.hpp"
#include "../src/StepperState.hpp"

#include <gmock/gmock.h>

class TestStepper : public IStepper
{
public:
	TestStepper(){};

	MOCK_METHOD(void, Init, (), (override));
	MOCK_METHOD(uint32_t, GetCurrentSpeed, (), (override));
	MOCK_METHOD(void, SetDirection, (bool direction), (override));
	MOCK_METHOD(void, Update, (), (override));
	MOCK_METHOD(bool, GetDirection, (), (override));
	MOCK_METHOD(bool, GetTargetDirection, (), (override));
	MOCK_METHOD(uint32_t, GetTargetSpeed, (), (override));
	MOCK_METHOD(void, SetSpeed, (uint32_t speed), (override));
	MOCK_METHOD(uint32_t, GetSetSpeed, (), (override));
	MOCK_METHOD(void, Enable, (), (override));
	MOCK_METHOD(void, Disable, (), (override));
	MOCK_METHOD(bool, IsEnabled, (), (override));
};

namespace Stepper
{
	class TestStepperState : public StepperState
	{
	public:
		TestStepperState(std::shared_ptr<IStepper> aStepper, std::shared_ptr<ITime> aTime) : StepperState(aStepper, aTime){};

		MOCK_METHOD(void, ProcessCommand, (std::shared_ptr<Command> command), (override));
		MOCK_METHOD(States, GetState, (), (override));
		MOCK_METHOD(void, Run, (), (override));
	};
}