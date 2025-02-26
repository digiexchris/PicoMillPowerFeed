#pragma once

#include "Common.hxx"
#include "Settings.hxx"
#include "Stepper.hxx"
#include <memory>

#include <stdint.h>

namespace PowerFeed
{

	enum class States
	{
		STOPPED,
		ACCELERATING,
		COASTING,
		DECELERATING,
		CHANGING_DIRECTION
	};

	struct Command
	{
		enum class Type
		{
			START,
			STOP,
			CHANGE_SPEED,
			CHANGE_ACCELERATION
		};

		Type type;
	};

	struct ChangeSpeed : Command
	{
		ChangeSpeed(uint32_t speed) : speed(speed) { type = Type::CHANGE_SPEED; }
		uint32_t speed;
	};

	struct ChangeAcceleration : Command
	{
		ChangeAcceleration(uint32_t acceleration, uint32_t deceleration) : acceleration(acceleration) { type = Type::CHANGE_ACCELERATION; }
		uint32_t acceleration;
	};

	struct Stop : Command
	{
		Stop() { type = Type::STOP; }
	};

	struct Start : Command
	{
		Start(bool aDirection, uint32_t aSpeed) : direction(aDirection), speed(aSpeed) { type = Type::START; }
		bool direction;
		uint32_t speed;
	};

	template <typename DerivedStepper>
	class StepperState
	{
	public:
		StepperState(SettingsManager *aSettings, StepperType<DerivedStepper> *aStepper)
		{
			mySettings = aSettings;
			myStepper = aStepper;
			myState = States::STOPPED;
		}

		void ProcessCommand(const Command &aCommand);
		States GetState() { return myState; }
		void Run();

	private:
		void ProcessStop();
		void ProcessNewSpeed(uint32_t speed);
		void ProcessStart(bool direction, uint32_t speed);

		States myState;
		uint32_t myRequestedSpeed;
		StepperType<DerivedStepper> *myStepper;
		SettingsManager *mySettings;
	};

} // namespace PowerFeed