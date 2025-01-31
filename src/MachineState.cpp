#include "MachineState.hpp"
#include "StepperState.hpp"
#include "config.hpp"
#include <memory>

namespace PicoMill
{

	void Machine::OnValueChange(std::shared_ptr<StateChange> aStateChange)
	{

		if (aStateChange->type == DeviceState::LEFT_HIGH || aStateChange->type == DeviceState::RIGHT_HIGH)
		{
			// Clear invalid states and ignore updates related to them
			if (IsStateSet(MachineState::LEFT) && IsStateSet(MachineState::RIGHT))
			{
				ClearState(MachineState::LEFT);
				ClearState(MachineState::RIGHT);
				return;
			}
		}

		switch (aStateChange->type)
		{
		case DeviceState::LEFT_HIGH:
			SetState(MachineState::LEFT);
			if (IsStateSet(MachineState::RAPID))
			{
				if (myRapidSpeed > 0)
				{
					std::shared_ptr<Command> command = std::make_shared<Start>(leftDir, myRapidSpeed);
					myStepperState->ProcessCommand(command);
				}
			}
			else
			{
				if (myNormalSpeed > 0)
				{
					std::shared_ptr<Command> command = std::make_shared<Start>(leftDir, myNormalSpeed);
					myStepperState->ProcessCommand(command);
				}
			}
			break;
		case DeviceState::LEFT_LOW:
			ClearState(MachineState::LEFT);
			if (!IsStateSet(MachineState::RIGHT))
			{
				std::shared_ptr<Command> command = std::make_shared<Stop>();
				myStepperState->ProcessCommand(command);
			}
			break;
		case DeviceState::RIGHT_HIGH:
			SetState(MachineState::RIGHT);
			if (IsStateSet(MachineState::RAPID))
			{
				if (myRapidSpeed > 0)
				{
					std::shared_ptr<Command> command = std::make_shared<Start>(rightDir, myRapidSpeed);
					myStepperState->ProcessCommand(command);
				}
			}
			else
			{
				if (myNormalSpeed > 0)
				{
					std::shared_ptr<Command> command = std::make_shared<Start>(rightDir, myNormalSpeed);
					myStepperState->ProcessCommand(command);
				}
			}
			break;
		case DeviceState::RIGHT_LOW:
			ClearState(MachineState::RIGHT);
			if (!IsStateSet(MachineState::LEFT))
			{
				std::shared_ptr<Command> command = std::make_shared<Stop>();
				myStepperState->ProcessCommand(command);
			}
			break;
		case DeviceState::RAPID_HIGH:
			SetState(MachineState::RAPID);
			if (IsStateSet(MachineState::LEFT) || IsStateSet(MachineState::RIGHT))
			{
				if (myRapidSpeed > 0)
				{
					std::shared_ptr<Command> command = std::make_shared<ChangeSpeed>(myRapidSpeed);
					myStepperState->ProcessCommand(command);
				}
			}
			break;
		case DeviceState::RAPID_LOW:
			ClearState(MachineState::RAPID);
			if (IsStateSet(MachineState::LEFT) || IsStateSet(MachineState::RIGHT))
			{
				if (myNormalSpeed > 0)
				{
					std::shared_ptr<Command> command = std::make_shared<ChangeSpeed>(myNormalSpeed);
					myStepperState->ProcessCommand(command);
				}
			}
			break;
		case DeviceState::ENCODER_CHANGED:
		{
			auto state = std::static_pointer_cast<Int8StateChange>(aStateChange);
			bool moving = IsStateSet(MachineState::LEFT) || IsStateSet(MachineState::RIGHT);
			int8_t increment = state->value;

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
					std::shared_ptr<Command> command = std::make_shared<ChangeSpeed>(myRapidSpeed);
					myStepperState->ProcessCommand(command);
				}
			}
			else if (IsStateSet(MachineState::ACCELERATION_HIGH))
			{
				myAcceleration += ENCODER_COUNTS_TO_ACCELERATION;
				std::shared_ptr<Command> command = std::make_shared<ChangeAcceleration>(myAcceleration, myAcceleration);
				myStepperState->ProcessCommand(command);
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
					std::shared_ptr<Command> command = std::make_shared<ChangeSpeed>(myNormalSpeed);
					myStepperState->ProcessCommand(command);
				}
			}
		}

		break;

		case DeviceState::UNITS_TOGGLE:
			myDisplay->ToggleUnits();
			break;

		case DeviceState::ACCELERATION_HIGH:
			SetState(MachineState::ACCELERATION_HIGH);
			break;

		case DeviceState::ACCELERATION_LOW:
			ClearState(MachineState::ACCELERATION_HIGH);
			break;
		}

		UpdateDisplay();
	}

	void Machine::UpdateDisplay()
	{
		myDisplay->Clear();
		myDisplay->DrawSpeed(IsStateSet(MachineState::RAPID) ? myRapidSpeed : myNormalSpeed);

		if (IsStateSet(MachineState::LEFT) && IsStateSet(MachineState::RAPID))
		{
			myDisplay->DrawRapidLeft();
		}
		else if (IsStateSet(MachineState::RIGHT) && IsStateSet(MachineState::RAPID))
		{
			myDisplay->DrawRapidRight();
		}
		else if (IsStateSet(MachineState::LEFT))
		{
			myDisplay->DrawMovingLeft();
		}
		else if (IsStateSet(MachineState::RIGHT))
		{
			myDisplay->DrawMovingRight();
		}
		else
		{
			myDisplay->DrawStopped();
		}
	}

} // namespace PicoMill