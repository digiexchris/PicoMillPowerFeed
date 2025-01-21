#include "State.hpp"
#include "Common.hpp"
#include "config.hpp"
#include <cstdlib>
#include <memory>
#include <sys/stat.h>

State::State(std::shared_ptr<IStepper> aStepper)
{
	myState = States::STOPPED;
	myStepper = aStepper; // std::make_shared<Stepper>(Stepper(stepPinStepper, dirPinStepper, enablePinStepper, 0, ACCELERATION, DECELERATION, pio0, 0));
	myRequestedSpeed = 0;
}

void State::ProcessStop()
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

void State::ProcessCommand(Command &aCommand)
{
	switch (aCommand.type)
	{
	case Command::Type::STOP:

		ProcessStop();
		break;
	case Command::Type::CHANGE_SPEED:

		ProcessNewSpeed(static_cast<ChangeSpeed &>(aCommand).speed);
		break;

	case Command::Type::START:
	{
		auto start = static_cast<Start &>(aCommand);
		ProcessStart(start.direction, start.speed);
	}

	break;
	}
}

void State::ProcessStart(bool direction, uint32_t speed)
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

void State::ProcessNewSpeed(uint32_t speed)
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

void State::Run()
{

	// due to precaching this, it will only be accurate to speeds within 4 steps
	auto mySpeed = myStepper->GetCurrentSpeed();
	auto myDirection = myStepper->GetDirection();
	auto mySetSpeed = myStepper->GetSetSpeed();
	auto myTargetSpeed = myStepper->GetTargetSpeed();
	auto myTargetDirection = myStepper->GetTargetDirection();

	switch (myState)
	{
	case States::CHANGING_DIRECTION:
		if (myDirection == myTargetDirection && mySpeed > 0)
		{
			myState = States::ACCELERATING;
		}
		break;
	case States::ACCELERATING:
		if (mySpeed == myTargetSpeed)
		{
			myState = States::COASTING;
		}
		break;
	case States::COASTING:
		if (mySpeed < myTargetSpeed)
		{
		}
		else if (mySpeed > myTargetSpeed)
		{
		}
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

		if (disableIdleTimeout >= 0)
		{

			if (myStepper->IsEnabled())
			{
				if (disableIdleTimeout > 0 && myStoppedAt + disableIdleTimeout < getCurrentTimeInMilliseconds())
				{
					myStepper->Disable();
					myStoppedAt = 0;
				}
				else
				{
					myStoppedAt = getCurrentTimeInMilliseconds();
				}
			}
		}

		return;
	}

	myStepper->Update();
}