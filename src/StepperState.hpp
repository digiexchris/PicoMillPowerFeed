#pragma once

#include "Common.hpp"
#include "Stepper.hpp"
// #include <memory>

#include <stdint.h>

namespace PicoMill
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

		virtual ~Command() = default;
	};

	struct ChangeSpeed : Command
	{
		ChangeSpeed(uint32_t aspeed) : speed(aspeed) { type = Type::CHANGE_SPEED; }
		uint32_t speed;
	};

	struct ChangeAcceleration : Command
	{
		ChangeAcceleration(uint32_t anacceleration) : acceleration(anacceleration) { type = Type::CHANGE_ACCELERATION; }
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
		StepperState(IStepper &aStepper, ITime &aTime) : myStepper(aStepper), myTime(aTime)
		{
			myState = States::STOPPED;
			myStoppedAt = 0;
		}

		virtual void ProcessCommand(Command &command);
		virtual States GetState() { return myState; }
		virtual void Run();

	private:
		void ProcessStop();
		void ProcessNewSpeed(uint32_t speed);
		void ProcessStart(bool direction, uint32_t speed);

		States myState;

		uint32_t myRequestedSpeed;

		uint64_t myStoppedAt;

		IStepper &myStepper;

		ITime &myTime;
	};

} // namespace Stepper