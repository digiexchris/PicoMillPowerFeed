#pragma once

#include "Display.hxx"
#include "Event.hxx"
#include "Settings.hxx"
#include "StepperState.hxx"
#include <cstdint>
#include <memory>

namespace PowerFeed
{

	enum class MachineState : uint8_t
	{
		LEFT = 1,
		RIGHT = 2,
		RAPID = 4,
		ACCELERATION_HIGH = 8,
	};

	enum class DeviceState : uint8_t
	{
		LEFT_HIGH,
		LEFT_LOW,
		RIGHT_HIGH,
		RIGHT_LOW,
		RAPID_HIGH,
		RAPID_LOW,
		ACCELERATION_HIGH,
		ACCELERATION_LOW,
		ENCODER_CHANGED,
		UNITS_TOGGLE
	};

	struct StateChange
	{
		DeviceState type;
		StateChange(DeviceState aType) : type(aType){};
	};

	struct BoolStateChange : StateChange
	{
		BoolStateChange(DeviceState aDeviceState) : StateChange{aDeviceState} {}
	};

	struct UInt32StateChange : StateChange
	{
		UInt32StateChange(DeviceState aDeviceState, uint32_t aValue) : StateChange{aDeviceState}, value(aValue) {}
		uint32_t value;
	};

	struct Int8StateChange : StateChange
	{
		Int8StateChange(DeviceState aDeviceState, int8_t aValue) : StateChange{aDeviceState}, value(aValue) {}
		int8_t value;
	};

	template <typename T>
	struct ValueChange : StateChange
	{
		ValueChange(DeviceState aDeviceState, T aValue) : StateChange{aDeviceState}, value(aValue) {}
		T value;
	};

	template <typename DerivedStepper>
	class Machine
	{
	public:
		Machine(SettingsManager *aSettings,
				Display *aDisplay,
				StepperState<DerivedStepper> *aStepperState,
				uint32_t aNormalSpeed = 1,
				uint32_t aRapidSpeed = 2)
			: mySettings(aSettings), myDisplay(aDisplay), myStepperState(aStepperState), myNormalSpeed(aNormalSpeed), myRapidSpeed(aRapidSpeed){};

		void OnValueChange(const StateChange &aStateChange)
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
				// if (!IsStateSet(MachineState::RIGHT))
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
				// if (!IsStateSet(MachineState::LEFT))
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

		bool IsStateSet(MachineState state) const
		{
			return (myState & static_cast<uint8_t>(state)) != 0;
		}

		Display *GetDisplay() const { return myDisplay; }

	private:
		Display *myDisplay;
		StepperState<DerivedStepper> *myStepperState;
		uint32_t myNormalSpeed = 1;
		uint32_t myRapidSpeed = 20000;
		uint32_t myAcceleration;
		uint8_t myState = 0;
		Units myUnits = Units::Millimeter;

		SettingsManager *mySettings;

		void UpdateDisplay()
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

		void SetState(MachineState state)
		{
			myState |= static_cast<uint8_t>(state);
		}

		void ClearState(MachineState state)
		{
			myState &= ~static_cast<uint8_t>(state);
		}
	};

} // namespace PowerFeed