#include "MachineState.hpp"
#include "StepperState.hpp"
#include "config.hpp"
#include <memory>

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
				std::shared_ptr<Stepper::Command> command = std::make_shared<Stepper::Start>(leftDir, myRapidSpeed);
				myStepperState->ProcessCommand(command);
			}
		}
		else
		{
			if (myNormalSpeed > 0)
			{
				std::shared_ptr<Stepper::Command> command = std::make_shared<Stepper::Start>(leftDir, myNormalSpeed);
				myStepperState->ProcessCommand(command);
			}
		}
		break;
	case DeviceState::LEFT_LOW:
		ClearState(MachineState::LEFT);
		if (!IsStateSet(MachineState::RIGHT))
		{
			std::shared_ptr<Stepper::Command> command = std::make_shared<Stepper::Stop>();
			myStepperState->ProcessCommand(command);
		}
		break;
	case DeviceState::RIGHT_HIGH:
		SetState(MachineState::RIGHT);
		if (IsStateSet(MachineState::RAPID))
		{
			if (myRapidSpeed > 0)
			{
				std::shared_ptr<Stepper::Command> command = std::make_shared<Stepper::Start>(rightDir, myRapidSpeed);
				myStepperState->ProcessCommand(command);
			}
		}
		else
		{
			if (myNormalSpeed > 0)
			{
				std::shared_ptr<Stepper::Command> command = std::make_shared<Stepper::Start>(rightDir, myNormalSpeed);
				myStepperState->ProcessCommand(command);
			}
		}
		break;
	case DeviceState::RIGHT_LOW:
		ClearState(MachineState::RIGHT);
		if (!IsStateSet(MachineState::LEFT))
		{
			std::shared_ptr<Stepper::Command> command = std::make_shared<Stepper::Stop>();
			myStepperState->ProcessCommand(command);
		}
		break;
	case DeviceState::RAPID_HIGH:
		SetState(MachineState::RAPID);
		if (IsStateSet(MachineState::LEFT) || IsStateSet(MachineState::RIGHT))
		{
			if (myRapidSpeed > 0)
			{
				std::shared_ptr<Stepper::Command> command = std::make_shared<Stepper::ChangeSpeed>(myRapidSpeed);
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
				std::shared_ptr<Stepper::Command> command = std::make_shared<Stepper::ChangeSpeed>(myNormalSpeed);
				myStepperState->ProcessCommand(command);
			}
		}
		break;
	case DeviceState::NORMAL_SPEED_CHANGE:
	{
		auto state = std::static_pointer_cast<UInt32StateChange>(aStateChange);

		if (myNormalSpeed != state->value)
		{
			myNormalSpeed = state->value;
			if (IsStateSet(MachineState::LEFT) || IsStateSet(MachineState::RIGHT))
			{
				std::shared_ptr<Stepper::Command> command = std::make_shared<Stepper::ChangeSpeed>(myNormalSpeed);
				myStepperState->ProcessCommand(command);
			}
		}
	}

	break;
	case DeviceState::RAPID_SPEED_CHANGE:
		myNormalSpeed = std::static_pointer_cast<UInt32StateChange>(aStateChange)->value;
		break;
	}
}