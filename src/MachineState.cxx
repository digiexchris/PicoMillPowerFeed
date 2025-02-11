#include "MachineState.hxx"
#include "StepperState.hxx"
#include "config.h"
#include <memory>

namespace PowerFeed
{

	Machine::Machine(std::shared_ptr<SettingsManager> aSettings, std::shared_ptr<Display> aDisplay, std::shared_ptr<StepperState> aStepperState, uint32_t aNormalSpeed, uint32_t aRapidSpeed) : mySettings(aSettings), myDisplay(aDisplay), myStepperState(aStepperState), myNormalSpeed(aNormalSpeed), myRapidSpeed(aRapidSpeed){};

	void Machine::OnValueChange(std::shared_ptr<StateChange> aStateChange)
	{
		std::shared_ptr<Settings> settings = mySettings->Get();
		Settings::Mechanical mechanical = settings->mechanical;
		Settings::Controls controls = settings->controls;

		// printf("Machine::OnValueChange: %u\n", (uint16_t)aStateChange->type);
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
					std::shared_ptr<Command> command = std::make_shared<Start>(mechanical.moveLeftDirection, myRapidSpeed);
					myStepperState->ProcessCommand(command);
				}
			}
			else
			{
				if (myNormalSpeed > 0)
				{
					std::shared_ptr<Command> command = std::make_shared<Start>(mechanical.moveLeftDirection, myNormalSpeed);
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
					std::shared_ptr<Command> command = std::make_shared<Start>(mechanical.moveRightDirection, myRapidSpeed);
					myStepperState->ProcessCommand(command);
				}
			}
			else
			{
				if (myNormalSpeed > 0)
				{
					std::shared_ptr<Command> command = std::make_shared<Start>(mechanical.moveRightDirection, myNormalSpeed);
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
			auto state = std::static_pointer_cast<ValueChange<int16_t>>(aStateChange);
			bool moving = IsStateSet(MachineState::LEFT) || IsStateSet(MachineState::RIGHT);
			int16_t increment = state->value;

			// Apply acceleration curve to encoder input
			if (abs(increment) > 16)
			{
				increment *= 10;
			}
			else if (abs(increment) > 32)
			{
				increment *= 50;
			}

			if (IsStateSet(MachineState::RAPID))
			{
				int32_t speed = static_cast<int32_t>(myRapidSpeed) + (increment * controls.encoderCountsToStepsPerSecond);

				if (speed < mechanical.accelerationJerk)
				{
					myRapidSpeed = mechanical.accelerationJerk;
				}
				else if (speed > mechanical.maxStepsPerSecond)
				{
					myRapidSpeed = mechanical.maxStepsPerSecond;
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
				myAcceleration += controls.encoderCountsToStepsPerSecond;
				std::shared_ptr<Command> command = std::make_shared<ChangeAcceleration>(myAcceleration, myAcceleration);
				myStepperState->ProcessCommand(command);
			}
			else
			{
				int32_t speed = static_cast<int32_t>(myNormalSpeed) + (increment * controls.encoderCountsToStepsPerSecond);

				if (speed < mechanical.accelerationJerk)
				{
					myNormalSpeed = mechanical.accelerationJerk;
				}
				else if (speed > mechanical.maxStepsPerSecond)
				{
					myNormalSpeed = mechanical.maxStepsPerSecond;
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

		// printf("Machine::OnValueChange: Display Updated\n");
	}

	void Machine::UpdateDisplay()
	{
		myDisplay->ClearBuffer();

		auto speed = IsStateSet(MachineState::RAPID) ? myRapidSpeed : myNormalSpeed;
		myDisplay->DrawSpeed(speed);

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

		myDisplay->Refresh();
	}

} // namespace PowerFeed