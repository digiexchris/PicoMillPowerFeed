#pragma once

#include "../src/Stepper.hxx"
#include "../src/StepperState.hxx"

#include <gmock/gmock.h>
#include <memory>
namespace PowerFeed
{

	namespace Drivers
	{
		class TestStepper : public Stepper<TestStepper>
		{
		public:
			MOCK_METHOD(bool, GetDirection, (), ());
			MOCK_METHOD(bool, GetTargetDirection, (), ());
			MOCK_METHOD(void, SetDirection, (bool aDirection), ());
			MOCK_METHOD(uint32_t, GetTargetSpeed, (), ());
			MOCK_METHOD(void, Stop, (), ());
			MOCK_METHOD(void, Start, (), ());
			MOCK_METHOD(void, SetSpeed, (uint32_t speed), ());
			MOCK_METHOD(void, Init, (), ());
			MOCK_METHOD(uint32_t, GetCurrentSpeed, (), ());
			MOCK_METHOD(bool, IsRunning, (), ());
			MOCK_METHOD(bool, Update, (), ());
		};

	}

	class TestStepperState : public StepperState<Drivers::TestStepper>
	{
	public:
		TestStepperState(SettingsManager *aSettings, Stepper<Drivers::TestStepper> *aStepper) : StepperState(aSettings, aStepper){};

		MOCK_METHOD(void, ProcessCommand, (const Command &command), (override));
		MOCK_METHOD(States, GetState, (), (override));
		MOCK_METHOD(void, Run, (), (override));
	};
}