#pragma once

#include "Display.hxx"
#include "Event.hxx"
#include "Helpers.hxx"
#include "Settings.hxx"
#include "Stepper.hxx"
#include "projdefs.h"
#include <cstdint>
#include <memory>

namespace PowerFeed
{

	enum class UIState : uint8_t
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
	class UI
	{
	public:
		UI(SettingsManager *aSettings,
		   Display *aDisplay,
		   StepperBase<DerivedStepper> *aStepper,
		   uint32_t aNormalSpeed = 1,
		   uint32_t aRapidSpeed = 2)
			: mySettings(aSettings), myDisplay(aDisplay), myStepper(aStepper)
		{
			// Initialize speeds from saved settings if available
			auto settings = mySettings->Get();
			if (settings)
			{
				myNormalSpeed = settings->savedSettings.normalSpeed;
				myRapidSpeed = settings->savedSettings.rapidSpeed;

				// Set display units based on saved setting
				if (settings->savedSettings.inchUnits && myDisplay->GetUnits() != Units::Inch)
				{
					myDisplay->SetUnits(Units::Inch);
				}
				else if (!settings->savedSettings.inchUnits && myDisplay->GetUnits() != Units::Millimeter)
				{
					myDisplay->SetUnits(Units::Millimeter);
				}
			}
			else
			{
				// Use provided defaults if settings aren't available
				myNormalSpeed = aNormalSpeed;
				myRapidSpeed = aRapidSpeed;
			}

			// Initialize the UI
			xTaskCreate(InitTask, "UI_Init", configMINIMAL_STACK_SIZE * 4, this, tskIDLE_PRIORITY + 1, nullptr);
		}

		static void InitTask(void *pvParameters)
		{
			UI *instance = static_cast<UI *>(pvParameters);

			while (instance->myDisplay->IsReady() == false)
			{
				vTaskDelay(pdMS_TO_TICKS(100));
			}

			instance->myDisplay->DrawStart();
			instance->myDisplay->WriteBuffer();
			instance->myDisplay->Refresh();

			vTaskDelay(pdMS_TO_TICKS(250));

			instance->UpdateDisplay();

			instance->myIsReady = true;

			ExitTask();
		}

		bool IsReady() const
		{
			return myIsReady;
		}

		void OnValueChange(const StateChange &aStateChange)
		{
			std::shared_ptr<Settings> settings = mySettings->Get();
			Settings::Mechanical mechanical = settings->mechanical;
			Settings::Controls controls = settings->controls;

			// printf("UI::OnValueChange: %u\n", (uint16_t)aStateChange.type);
			if (aStateChange.type == DeviceState::LEFT_HIGH || aStateChange.type == DeviceState::RIGHT_HIGH)
			{
				// Clear invalid states and ignore updates related to them
				if (IsStateSet(UIState::LEFT) && IsStateSet(UIState::RIGHT))
				{
					ClearState(UIState::LEFT);
					ClearState(UIState::RIGHT);
					myStepper->Stop();
					return;
				}
			}

			switch (aStateChange.type)
			{
			case DeviceState::LEFT_HIGH:
			{
				SetState(UIState::LEFT);
				if (IsStateSet(UIState::RAPID))
				{
					myStepper->SetSpeed(myRapidSpeed);
				}
				else
				{
					myStepper->SetSpeed(myNormalSpeed);
				}

				if (myStepper->IsRunning())
				{
					if (myStepper->IsStopping())
					{
						if (myStepper->GetDirection() == mySettings->Get()->mechanical.moveLeftDirection)
						{
							// it's stopping, but we want to resume in the same direction, it's safe to restart
							myStepper->Start();
						}
					}
				}
				else
				{
					// it was stopped, we can start with any direction
					myStepper->SetDirection(mySettings->Get()->mechanical.moveLeftDirection);
					myStepper->Start();
				}
			}

			break;
			case DeviceState::LEFT_LOW:
				ClearState(UIState::LEFT);
				if (!IsStateSet(UIState::RIGHT))
				{
					myStepper->Stop();
				}
				break;
			case DeviceState::RIGHT_HIGH:
				SetState(UIState::RIGHT);
				if (IsStateSet(UIState::RAPID))
				{
					myStepper->SetSpeed(myRapidSpeed);
				}
				else
				{
					myStepper->SetSpeed(myNormalSpeed);
				}

				if (myStepper->IsRunning())
				{
					if (myStepper->IsStopping())
					{
						if (myStepper->GetDirection() == mySettings->Get()->mechanical.moveRightDirection)
						{
							// it's stopping, but we want to resume in the same direction, it's safe to restart
							myStepper->Start();
						}
					}
				}
				else
				{
					// it was stopped, we can start with any direction
					myStepper->SetDirection(mySettings->Get()->mechanical.moveRightDirection);
					myStepper->Start();
				}
				break;
			case DeviceState::RIGHT_LOW:
				ClearState(UIState::RIGHT);
				if (!IsStateSet(UIState::LEFT))
				{
					myStepper->Stop();
				}
				break;
			case DeviceState::RAPID_HIGH:
				SetState(UIState::RAPID);
				if (IsStateSet(UIState::LEFT) || IsStateSet(UIState::RIGHT))
				{
					myStepper->SetSpeed(myRapidSpeed);
				}
				break;
			case DeviceState::RAPID_LOW:
				ClearState(UIState::RAPID);
				if (IsStateSet(UIState::LEFT) || IsStateSet(UIState::RIGHT))
				{
					myStepper->SetSpeed(myNormalSpeed);
				}
				break;
			case DeviceState::ENCODER_CHANGED:
			{
				const ValueChange<int16_t> &state = static_cast<const ValueChange<int16_t> &>(aStateChange);
				bool moving = IsStateSet(UIState::LEFT) || IsStateSet(UIState::RIGHT);
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

				if (IsStateSet(UIState::RAPID))
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
						myStepper->SetSpeed(myRapidSpeed);
					}

					// Save rapid speed to settings when changed
					mySettings->Set("RAPID_SPEED", myRapidSpeed);
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
						myStepper->SetSpeed(myNormalSpeed);
					}

					// Save normal speed to settings when changed
					mySettings->Set("NORMAL_SPEED", myNormalSpeed);
				}
			}

			break;

			case DeviceState::UNITS_TOGGLE:
				myDisplay->ToggleUnits();
				// Save units setting when changed
				mySettings->Set("INCH_UNITS", myDisplay->GetUnits() == Units::Inch);
				break;
			}

			UpdateDisplay();
		}

		bool IsStateSet(UIState state) const
		{
			return (myState & static_cast<uint8_t>(state)) != 0;
		}

		Display *GetDisplay() const { return myDisplay; }

	private:
		Display *myDisplay;
		StepperBase<DerivedStepper> *myStepper;
		uint32_t myNormalSpeed = 1;
		uint32_t myRapidSpeed = 20000;
		uint32_t myAcceleration;
		uint8_t myState = 0;
		Units myUnits = Units::Millimeter;

		SettingsManager *mySettings;

		bool myIsReady = false;

		void UpdateDisplay()
		{
			myDisplay->ClearBuffer();

			auto speed = IsStateSet(UIState::RAPID) ? myRapidSpeed : myNormalSpeed;
			myDisplay->DrawSpeed(speed);

			if (IsStateSet(UIState::LEFT) && IsStateSet(UIState::RAPID))
			{
				myDisplay->DrawRapidLeft();
			}
			else if (IsStateSet(UIState::RIGHT) && IsStateSet(UIState::RAPID))
			{
				myDisplay->DrawRapidRight();
			}
			else if (IsStateSet(UIState::LEFT))
			{
				myDisplay->DrawMovingLeft();
			}
			else if (IsStateSet(UIState::RIGHT))
			{
				myDisplay->DrawMovingRight();
			}
			else
			{
				myDisplay->DrawStopped();
			}

			myDisplay->Refresh();
		}

		void SetState(UIState state)
		{
			myState |= static_cast<uint8_t>(state);
		}

		void ClearState(UIState state)
		{
			myState &= ~static_cast<uint8_t>(state);
		}
	};

} // namespace PowerFeed
