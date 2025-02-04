#include "StepperState.hpp"
#include "Common.hpp"
#include "config.hpp"
#include <memory.h>
#include <stdlib.h>
#include <sys/stat.h>

namespace PicoMill
{

	void StepperState::ProcessStop()
	{
		myStepper->SetSpeed(0);

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

	void StepperState::ProcessCommand(std::shared_ptr<Command> aCommand)
	{
		switch (aCommand->type)
		{
		case Command::Type::STOP:

			ProcessStop();
			break;
		case Command::Type::CHANGE_SPEED:
		{
			std::shared_ptr<ChangeSpeed> changeSpeed = std::static_pointer_cast<ChangeSpeed>(aCommand);
			ProcessNewSpeed(changeSpeed->speed);
		}
		break;

		case Command::Type::CHANGE_ACCELERATION:
		{
			std::shared_ptr<ChangeAcceleration> changeAcceleration = std::static_pointer_cast<ChangeAcceleration>(aCommand);
			myStepper->SetAcceleration(changeAcceleration->acceleration);
		}

		case Command::Type::START:
		{
			std::shared_ptr<Start> start = std::static_pointer_cast<Start>(aCommand);
			ProcessStart(start->direction, start->speed);
		}

		break;
		}
	}

	void StepperState::ProcessStart(bool direction, uint32_t speed)
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
	}

	void StepperState::ProcessNewSpeed(uint32_t speed)
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

	void StepperState::Run()
	{

		// due to precaching this, it will only be accurate to speeds within 4 steps or whatever the stepper's steps per update is
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

			if (myTargetSpeed == 0)
			{

				if (disableIdleTimeout >= 0)
				{

					if (myStepper->IsEnabled())
					{
						const uint64_t currentTime = myTime->GetCurrentTimeInMilliseconds();
						if (disableIdleTimeout > 0 && myStoppedAt + disableIdleTimeout < currentTime)
						{
							myStepper->Disable();
							myStoppedAt = 0;
						}
						else
						{
							myStoppedAt = currentTime;
						}
					}
				}
			}

			return;
		}
	}

} // namespace Stepper