#include "MachineState.hxx"
#include "StepperState.hxx"
// #include "drivers/stepper/PicoStepper.hxx"

namespace PowerFeed
{
	template <typename DerivedStepper>
	Machine<DerivedStepper>::Machine(SettingsManager *aSettings, Display *aDisplay, StepperState<DerivedStepper> *aStepperState, uint32_t aNormalSpeed, uint32_t aRapidSpeed)
		: mySettings(aSettings), myDisplay(aDisplay), myStepperState(aStepperState), myNormalSpeed(aNormalSpeed), myRapidSpeed(aRapidSpeed){};

	template <typename DerivedStepper>
	void Machine<DerivedStepper>::OnValueChange(const StateChange &aStateChange)
	{
		std::shared_ptr<Settings> settings = mySettings->Get();
		Settings::Mechanical mechanical = settings->mechanical;
		Settings::Controls controls = settings->controls;

		// printf("Machine::OnValueChange: %u\n", (uint16_t)aStateChange.type);
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
					// Create command object on stack and pass by reference
					Start command(mechanical.moveLeftDirection, myRapidSpeed);
					myStepperState->ProcessCommand(command);
				}
			}
			else
			{
				if (myNormalSpeed > 0)
				{
					Start command(mechanical.moveLeftDirection, myNormalSpeed);
					myStepperState->ProcessCommand(command);
				}
			}
			break;
		case DeviceState::LEFT_LOW:
			ClearState(MachineState::LEFT);
			if (!IsStateSet(MachineState::RIGHT))
			{
				Stop command;
				myStepperState->ProcessCommand(command);
			}
			break;
		case DeviceState::RIGHT_HIGH:
			SetState(MachineState::RIGHT);
			if (IsStateSet(MachineState::RAPID))
			{
				if (myRapidSpeed > 0)
				{
					Start command(mechanical.moveRightDirection, myRapidSpeed);
					myStepperState->ProcessCommand(command);
				}
			}
			else
			{
				if (myNormalSpeed > 0)
				{
					Start command(mechanical.moveRightDirection, myNormalSpeed);
					myStepperState->ProcessCommand(command);
				}
			}
			break;
		case DeviceState::RIGHT_LOW:
			ClearState(MachineState::RIGHT);
			if (!IsStateSet(MachineState::LEFT))
			{
				Stop command;
				myStepperState->ProcessCommand(command);
			}
			break;
		case DeviceState::RAPID_HIGH:
			SetState(MachineState::RAPID);
			if (IsStateSet(MachineState::LEFT) || IsStateSet(MachineState::RIGHT))
			{
				if (myRapidSpeed > 0)
				{
					ChangeSpeed command(myRapidSpeed);
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
					ChangeSpeed command(myNormalSpeed);
					myStepperState->ProcessCommand(command);
				}
			}
			break;
		case DeviceState::ENCODER_CHANGED:
		{
			const ValueChange<int16_t> &state = static_cast<const ValueChange<int16_t> &>(aStateChange);
			bool moving = IsStateSet(MachineState::LEFT) || IsStateSet(MachineState::RIGHT);
			int16_t increment = state.value;

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
					ChangeSpeed command(myRapidSpeed);
					myStepperState->ProcessCommand(command);
				}
			}
			else if (IsStateSet(MachineState::ACCELERATION_HIGH))
			{
				myAcceleration += controls.encoderCountsToStepsPerSecond;
				ChangeAcceleration command(myAcceleration, myAcceleration);
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
					ChangeSpeed command(myNormalSpeed);
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

	template <typename DerivedStepper>
	void Machine<DerivedStepper>::UpdateDisplay()
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

	// template class Machine<PowerFeed::Drivers::PicoStepper>;

} // namespace PowerFeed