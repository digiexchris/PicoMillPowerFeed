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

	class StepperState
	{
	public:
		StepperState(std::shared_ptr<SettingsManager> aSettings, std::shared_ptr<Stepper> aStepper, std::shared_ptr<ITime> aTime) : mySettings(aSettings), myStepper(aStepper), myTime(aTime)
		{
			myState = States::STOPPED;
			myStoppedAt = 0;
		}

		virtual void ProcessCommand(std::shared_ptr<Command> command);
		virtual States GetState() { return myState; }
		virtual void Run();

	private:
		void ProcessStop();
		void ProcessNewSpeed(uint32_t speed);
		void ProcessStart(bool direction, uint32_t speed);

		States myState;

		uint32_t myRequestedSpeed;

		uint64_t myStoppedAt;

		std::shared_ptr<Stepper> myStepper;

		std::shared_ptr<ITime> myTime;

		std::shared_ptr<SettingsManager> mySettings;
	};

} // namespace Stepper