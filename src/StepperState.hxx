#pragma once

#include "Common.hxx"
#include "Settings.hxx"
#include "Stepper.hxx"
#include <cstdlib>
#include <memory>
#include <sys/stat.h>

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
			myTargetDirection = false;
			myRequestedSpeed = 0;
		}

		virtual void ProcessCommand(const Command &aCommand);
		virtual States GetState() { return myState; }
		virtual void Run();

	private:
		void ProcessStop();
		void ProcessNewSpeed(uint32_t speed);
		void ProcessStart(bool direction, uint32_t speed);

		States myState;
		uint32_t myRequestedSpeed;
		bool myTargetDirection;
		StepperType<DerivedStepper> *myStepper;
		SettingsManager *mySettings;
	};

	// Template implementations

	template <typename DerivedStepper>
	void StepperState<DerivedStepper>::ProcessStop()
	{
		myStepper->Stop();

		switch (myState)
		{
		case States::CHANGING_DIRECTION:
		case States::ACCELERATING:
		case States::COASTING:

			myState = States::DECELERATING;
			break;
		case States::DECELERATING:
			return;
			break;
		case States::STOPPED:
			return;
		}
	}

	template <typename DerivedStepper>
	void StepperState<DerivedStepper>::ProcessCommand(const Command &aCommand)
	{
		switch (aCommand.type)
		{
		case Command::Type::STOP:

			ProcessStop();
			break;
		case Command::Type::CHANGE_SPEED:
		{
			const ChangeSpeed &changeSpeed = static_cast<const ChangeSpeed &>(aCommand);
			ProcessNewSpeed(changeSpeed.speed);
		}
		break;

		case Command::Type::CHANGE_ACCELERATION:
		{
			const ChangeAcceleration &changeAcceleration = static_cast<const ChangeAcceleration &>(aCommand);
			assert("Not implemented");
		}
		break;

		case Command::Type::START:
		{
			const Start &start = static_cast<const Start &>(aCommand);
			ProcessStart(start.direction, start.speed);
		}
		break;
		}
	}

	template <typename DerivedStepper>
	void StepperState<DerivedStepper>::ProcessStart(bool direction, uint32_t speed)
	{
		auto mySpeed = myStepper->GetCurrentSpeed();
		auto myDirection = myStepper->GetDirection();
		auto myTargetSpeed = myStepper->GetTargetSpeed();
		auto myTargetDirection = myStepper->GetTargetDirection();

		switch (myState)
		{
		case States::ACCELERATING:

			if (myDirection != direction)
			{
				myState = States::CHANGING_DIRECTION;
				myRequestedSpeed = speed;
				// Don't set direction yet, just stop and wait for Run() to handle it
				myStepper->Stop();
			}
			else if (speed > mySpeed)
			{
				myState = States::ACCELERATING;
			}
			else if (speed < mySpeed)
			{
				myState = States::DECELERATING;
			}
			else if (speed == mySpeed)
			{
				myState = States::COASTING;
			}

			if (mySpeed != speed)
			{
				myStepper->SetSpeed(speed);
			}
			break;
		case States::COASTING:
			if (myDirection != direction)
			{
				myState = States::CHANGING_DIRECTION;
				myRequestedSpeed = speed;
				// Don't set direction yet, just stop and wait for Run() to handle it
				myStepper->Stop();
			}
			else if (speed > mySpeed)
			{
				myStepper->SetSpeed(speed);
				myState = States::ACCELERATING;
			}
			else if (speed < mySpeed)
			{
				myStepper->SetSpeed(speed);
				myState = States::DECELERATING;
			}
			else
			{
				return;
			}

			break;
		case States::DECELERATING:
			if (myDirection != direction)
			{
				myState = States::CHANGING_DIRECTION;
				myRequestedSpeed = speed;
				// Don't set direction yet, just stop and wait for Run() to handle it
				myStepper->Stop();
			}
			else if (speed > mySpeed)
			{
				myState = States::ACCELERATING;
			}
			else if (speed < mySpeed)
			{
				// NOOP, already decelerating
			}
			else
			{
				myState = States::COASTING;
			}

			if (mySpeed != speed)
			{
				myStepper->SetSpeed(speed);
			}
			break;
		case States::STOPPED:
			if (myDirection != direction)
			{
				myStepper->SetDirection(direction);
			}

			if (speed != 0)
			{

				myState = States::ACCELERATING;
				myStepper->SetSpeed(speed);
			}

			break;
		case States::CHANGING_DIRECTION:
			bool willChangeDirection = false;
			if (myDirection != direction)
			{
				// Don't change direction while in CHANGING_DIRECTION state
				// Just update the requested direction for when stepper stops
				myRequestedSpeed = speed;
				myTargetDirection = direction;
				willChangeDirection = true;
			}

			if (!willChangeDirection)
			{
				if (speed > mySpeed)
				{
					myState = States::ACCELERATING;
				}
				else if (speed < mySpeed)
				{
					// NOOP, already decelerating
				}
				else
				{
					myState = States::COASTING;
				}
			}

			if (mySpeed != speed)
			{
				myStepper->SetSpeed(speed);
			}
			break;
		}

		if (!myStepper->IsRunning())
		{
			myStepper->Start();
		}
	}

	template <typename DerivedStepper>
	void StepperState<DerivedStepper>::ProcessNewSpeed(uint32_t speed)
	{
		auto mySpeed = myStepper->GetCurrentSpeed();
		auto myDirection = myStepper->GetDirection();
		auto myTargetSpeed = myStepper->GetTargetSpeed();
		auto myTargetDirection = myStepper->GetTargetDirection();

		switch (myState)
		{
		case States::CHANGING_DIRECTION:
			if (speed != mySpeed)
			{
				myStepper->SetSpeed(speed);
			}
		case States::ACCELERATING:
			if (speed < mySpeed)
			{
				myState = States::DECELERATING;
			}

			if (mySpeed != speed)
			{
				myStepper->SetSpeed(speed);
			}
			break;
		case States::COASTING:
			if (speed < mySpeed)
			{
				myStepper->SetSpeed(speed);
				myState = States::DECELERATING;
			}
			else if (speed > mySpeed)
			{
				myStepper->SetSpeed(speed);
				myState = States::ACCELERATING;
			}
			else
			{
				return;
			}
			break;
		case States::DECELERATING:
			if (speed > mySpeed)
			{
				myStepper->SetSpeed(speed);
				myState = States::ACCELERATING;
			}
			else if (speed == mySpeed)
			{
				myStepper->SetSpeed(speed);
				myState = States::COASTING;
			}
			else
			{
				myStepper->SetSpeed(speed);
			}
			break;
		case States::STOPPED:
			myState = States::ACCELERATING;
			myStepper->SetSpeed(speed);
			break;
		}
	}

	template <typename DerivedStepper>
	void StepperState<DerivedStepper>::Run()
	{
		auto mySpeed = myStepper->GetCurrentSpeed();
		auto myDirection = myStepper->GetDirection();
		auto myTargetSpeed = myStepper->GetTargetSpeed();
		auto myTargetDirection = myStepper->GetTargetDirection();

		switch (myState)
		{
		case States::CHANGING_DIRECTION:
			// Check if the stepper has completely stopped
			if (mySpeed == 0)
			{
				// Now it's safe to change direction
				myStepper->SetDirection(myTargetDirection);
				
				// Now start again with the requested speed
				if (myRequestedSpeed > 0)
				{
					myStepper->SetSpeed(myRequestedSpeed);
					myStepper->Start();
					myState = States::ACCELERATING;
				}
				else
				{
					myState = States::STOPPED;
				}
			}
			break;
		case States::ACCELERATING:
			if (mySpeed == myTargetSpeed)
			{
				myState = States::COASTING;
			}
			break;
		case States::COASTING:
			// I think this is a NOOP since you should not see a target speed change without a command
			break;
		case States::DECELERATING:
			if (mySpeed == myTargetSpeed)
			{
				if (mySpeed == 0)
				{
					myState = States::STOPPED;
				}
				else
				{
					myState = States::COASTING;
				}
			}
			break;
		case States::STOPPED:
			break;
		}
	}

} // namespace PowerFeed