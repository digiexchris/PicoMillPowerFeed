#pragma once

#include "../src/Stepper.hxx"
#include "../src/StepperState.hxx"

#include <gmock/gmock.h>
namespace PowerFeed
{

	namespace Drivers
	{
		class TestStepper : public IStepper
		{
		public:
			MOCK_METHOD(void, SetAcceleration, (uint32_t acceleration), (override));
			MOCK_METHOD(uint32_t, GetCurrentSpeed, (), (override));
			MOCK_METHOD(void, SetDirection, (bool direction), (override));
			MOCK_METHOD(bool, GetDirection, (), (override));
			MOCK_METHOD(bool, GetTargetDirection, (), (override));
			MOCK_METHOD(uint32_t, GetTargetSpeed, (), (override));
			MOCK_METHOD(void, SetSpeed, (uint32_t speed), (override));
			MOCK_METHOD(uint32_t, GetSetSpeed, (), (override));
			MOCK_METHOD(void, Enable, (), (override));
			MOCK_METHOD(void, Disable, (), (override));
			MOCK_METHOD(bool, IsEnabled, (), (override));
			MOCK_METHOD(void, Start, (), (override));
		};

	}

	class TestStepperState : public StepperState
	{
	public:
		TestStepperState(std::shared_ptr<IStepper> aStepper, std::shared_ptr<ITime> aTime) : StepperState(aStepper, aTime){};

		MOCK_METHOD(void, ProcessCommand, (std::shared_ptr<Command> command), (override));
		MOCK_METHOD(States, GetState, (), (override));
		MOCK_METHOD(void, Run, (), (override));
	};
}