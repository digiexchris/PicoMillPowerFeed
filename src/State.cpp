#include "State.hpp"
#include "config.hpp"
#include <memory>
#include <sys/stat.h>

State::State(std::shared_ptr<IStepper> aStepper)
{
	myState = States::STOPPED;
	mySpeed = 0;
	myDirection = false;
	myTargetSpeed = 0;
	myTargetDirection = false;
	myStepper = aStepper; // std::make_shared<Stepper>(Stepper(stepPinStepper, dirPinStepper, enablePinStepper, 0, ACCELERATION, DECELERATION, pio0, 0));
}

void State::ProcessStop()
{
	switch (myState)
	{
	case States::ACCELERATING:
		myState = States::DECELERATING;
		break;
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

void State::ProcessCommand(Command aCommand)
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

	switch (myState)
	{
	case States::ACCELERATING:
	case States::COASTING:
	case States::DECELERATING:
		if (myDirection != direction)
		{
			myTargetSpeed = speed;
			myTargetDirection = direction;
			myState = States::DECELERATING;
		}
		else
		{
			switch (myState)
			{
			case States::ACCELERATING:
				if (speed > mySpeed)
				{
					myTargetSpeed = speed;
				}
				else if (speed < mySpeed)
				{
					myTargetSpeed = mySpeed;
					myState = States::DECELERATING;
				}
				else
				{
					myTargetSpeed = mySpeed;
					myState = States::COASTING;
				}
				break;
			case States::COASTING:
				if (speed > mySpeed)
				{
					myTargetSpeed = speed;
					myState = States::ACCELERATING;
				}
				else if (speed < mySpeed)
				{
					myTargetSpeed = speed;
					myState = States::DECELERATING;
				}
				else
				{
					return;
				}
				break;
			case States::DECELERATING:
				if (speed > mySpeed)
				{
					myTargetSpeed = speed;
					myState = States::ACCELERATING;
				}
				else if (speed < mySpeed)
				{
					myTargetSpeed = speed;
				}
				else
				{
					myTargetSpeed = speed;
					myState = States::COASTING;
				}
				break;
			case States::STOPPED:
				break;
			}
		}
		return;
		break;
	case States::STOPPED:
		myState = States::ACCELERATING;
		break;
	}
}

void State::ProcessNewSpeed(uint32_t speed)
{
	switch (myState)
	{
	case States::ACCELERATING:
		if (speed < mySpeed)
		{
			myState = States::DECELERATING;
		}
		myTargetSpeed = speed;
		break;
	case States::COASTING:
		if (speed < mySpeed)
		{
			myTargetSpeed = speed;
			myState = States::DECELERATING;
		}
		else if (speed > mySpeed)
		{
			myTargetSpeed = speed;
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
			myTargetSpeed = speed;
			myState = States::ACCELERATING;
		}
		else if (speed == mySpeed)
		{
			myTargetSpeed = speed;
			myState = States::COASTING;
		}
		else
		{
			myTargetSpeed = speed;
		}
		break;
	case States::STOPPED:
		myState = States::ACCELERATING;
		myTargetSpeed = speed;
		break;
	}
}

void State::Run()
{
	switch (myState)
	{
	case States::ACCELERATING:
		if (mySpeed == myTargetSpeed)
		{
			myState = States::COASTING;
		}
		else
		{
		}
		break;
	case States::COASTING:
		if (mySpeed < myTargetSpeed)
		{
			mySpeed++;
		}
		else if (mySpeed > myTargetSpeed)
		{
			mySpeed--;
		}
		break;
	case States::DECELERATING:
		mySpeed--;
		if (mySpeed == myTargetSpeed)
		{
			myState = States::COASTING;
		}
		break;
	case States::STOPPED:
		return;
	}
}