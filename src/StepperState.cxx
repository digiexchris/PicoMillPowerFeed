#include "StepperState.hxx"
#include "Common.hxx"
#include "Settings.hxx"
#include <cstdlib>
#include <sys/stat.h>

namespace PowerFeed
{
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
				myStepper->SetDirection(direction);
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
				myStepper->SetDirection(direction);
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
				myStepper->SetDirection(direction);
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
				myStepper->SetDirection(direction);
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
			if (myDirection == myTargetDirection && mySpeed > 0)
			{
				if (mySpeed < myTargetSpeed)
				{
					myState = States::ACCELERATING;
				}
				else if (mySpeed == myTargetSpeed)
				{
					myState = States::COASTING;
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