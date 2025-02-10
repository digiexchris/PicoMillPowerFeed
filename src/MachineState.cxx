#include "MachineState.hxx"
#include "StepperState.hxx"
#include "config.hxx"
// #include "pico/stdio.h"
// #include <memory>
#include <stdio.h>

namespace PowerFeed
{
	template <typename DISPLAY>
	Machine<DISPLAY>::Machine(Display<DISPLAY> &aDisplay, StepperState &aStepperState, uint32_t aNormalSpeed, uint32_t aRapidSpeed) : myDisplay(aDisplay), myStepperState(aStepperState), myNormalSpeed(aNormalSpeed), myRapidSpeed(aRapidSpeed){};

	template <typename DISPLAY>
	void Machine<DISPLAY>::OnValueChange(StateChange &aStateChange)
	{
		printf("Machine::OnValueChange: %u\n", (uint16_t)aStateChange.type);
		if (aStateChange.type == DeviceState::LEFT_HIGH || aStateChange.type == DeviceState::RIGHT_HIGH)
		{
			// Clear invalid states and ignore updates related to them
			if (IsStateSet(MachineState::LEFT) && IsStateSet(MachineState::RIGHT))
			{
				ClearState(MachineState::LEFT);
				ClearState(MachineState::RIGHT);
				return;
			}
		}

		switch (aStateChange.type)
		{
		case DeviceState::LEFT_HIGH:
			SetState(MachineState::LEFT);
			if (IsStateSet(MachineState::RAPID))
			{
				if (myRapidSpeed > 0)
				{
					Command command = Start(leftDir, myRapidSpeed);
					myStepperState.ProcessCommand(command);
				}
			}
			else
			{
				if (myNormalSpeed > 0)
				{
					Command command = Start(leftDir, myNormalSpeed);
					myStepperState.ProcessCommand(command);
				}
			}
			break;
		case DeviceState::LEFT_LOW:
			ClearState(MachineState::LEFT);
			if (!IsStateSet(MachineState::RIGHT))
			{
				Command command = Stop();
				myStepperState.ProcessCommand(command);
			}
			break;
		case DeviceState::RIGHT_HIGH:
			SetState(MachineState::RIGHT);
			if (IsStateSet(MachineState::RAPID))
			{
				if (myRapidSpeed > 0)
				{
					Command command = Start(rightDir, myRapidSpeed);
					myStepperState.ProcessCommand(command);
				}
			}
			else
			{
				if (myNormalSpeed > 0)
				{
					Command command = Start(rightDir, myNormalSpeed);
					myStepperState.ProcessCommand(command);
				}
			}
			break;
		case DeviceState::RIGHT_LOW:
			ClearState(MachineState::RIGHT);
			if (!IsStateSet(MachineState::LEFT))
			{
				Command command = Stop();
				myStepperState.ProcessCommand(command);
			}
			break;
		case DeviceState::RAPID_HIGH:
			SetState(MachineState::RAPID);
			if (IsStateSet(MachineState::LEFT) || IsStateSet(MachineState::RIGHT))
			{
				if (myRapidSpeed > 0)
				{
					Command command = ChangeSpeed(myRapidSpeed);
					myStepperState.ProcessCommand(command);
				}
			}
			break;
		case DeviceState::RAPID_LOW:
			ClearState(MachineState::RAPID);
			if (IsStateSet(MachineState::LEFT) || IsStateSet(MachineState::RIGHT))
			{
				if (myNormalSpeed > 0)
				{
					Command command = ChangeSpeed(myNormalSpeed);
					myStepperState.ProcessCommand(command);
				}
			}
			break;
		case DeviceState::ENCODER_CHANGED:
		{
			auto state = static_cast<Int8StateChange &>(aStateChange);
			bool moving = IsStateSet(MachineState::LEFT) || IsStateSet(MachineState::RIGHT);
			int8_t increment = state.value;

			if (IsStateSet(MachineState::RAPID))
			{
				int32_t speed = static_cast<int32_t>(myRapidSpeed) + (increment * ENCODER_COUNTS_TO_STEPS_PER_SECOND);

				if (speed < ACCELERATION_JERK)
				{
					myRapidSpeed = ACCELERATION_JERK;
				}
				else
				{
					myRapidSpeed = speed;
				}

				if (moving)
				{
					Command command = ChangeSpeed(myRapidSpeed);
					myStepperState.ProcessCommand(command);
				}
			}
			else if (IsStateSet(MachineState::ACCELERATION_HIGH))
			{
				myAcceleration += ENCODER_COUNTS_TO_ACCELERATION;
				ChangeAcceleration command(myAcceleration);
				myStepperState.ProcessCommand(command);
			}
			else
			{
				int32_t speed = static_cast<int32_t>(myNormalSpeed) + (increment * ENCODER_COUNTS_TO_STEPS_PER_SECOND);

				if (speed < ACCELERATION_JERK)
				{
					myNormalSpeed = ACCELERATION_JERK;
				}
				else
				{
					myNormalSpeed = speed;
				}

				if (moving)
				{
					Command command = ChangeSpeed(myNormalSpeed);
					myStepperState.ProcessCommand(command);
				}
			}
		}

		break;

		case DeviceState::UNITS_TOGGLE:
			myDisplay.ToggleUnits();
			break;

		case DeviceState::ACCELERATION_HIGH:
			SetState(MachineState::ACCELERATION_HIGH);
			break;

		case DeviceState::ACCELERATION_LOW:
			ClearState(MachineState::ACCELERATION_HIGH);
			break;
		}

		UpdateDisplay();

		printf("Machine::OnValueChange: Display Updated\n");
	}

	template <typename DISPLAY>
	void Machine<DISPLAY>::UpdateDisplay()
	{
		myDisplay.ClearBuffer();
		myDisplay.WriteBuffer();

		auto speed = IsStateSet(MachineState::RAPID) ? myRapidSpeed : myNormalSpeed;
		myDisplay.DrawSpeed(speed);

		if (IsStateSet(MachineState::LEFT) && IsStateSet(MachineState::RAPID))
		{
			myDisplay.DrawRapidLeft();
		}
		else if (IsStateSet(MachineState::RIGHT) && IsStateSet(MachineState::RAPID))
		{
			myDisplay.DrawRapidRight();
		}
		else if (IsStateSet(MachineState::LEFT))
		{
			myDisplay.DrawMovingLeft();
		}
		else if (IsStateSet(MachineState::RIGHT))
		{
			myDisplay.DrawMovingRight();
		}
		else
		{
			myDisplay.DrawStopped();
		}

		myDisplay.WriteBuffer();
	}

} // namespace PowerFeed