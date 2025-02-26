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
		Machine(std::shared_ptr<SettingsManager> aSettings,
				std::shared_ptr<Display> aDisplay,
				std::shared_ptr<StepperState<DerivedStepper>> aStepperState,
				uint32_t aNormalSpeed = 1,
				uint32_t aRapidSpeed = 2);

		void OnValueChange(std::shared_ptr<StateChange> anStateChange);

		bool IsStateSet(MachineState state) const
		{
			return (myState & static_cast<uint8_t>(state)) != 0;
		}

		std::shared_ptr<Display> GetDisplay() const { return myDisplay; }

	private:
		std::shared_ptr<Display> myDisplay;
		std::shared_ptr<StepperState<DerivedStepper>> myStepperState;
		uint32_t myNormalSpeed = 1;
		uint32_t myRapidSpeed = 20000;
		uint32_t myAcceleration;
		uint8_t myState = 0;
		Units myUnits = Units::Millimeter;

		std::shared_ptr<SettingsManager> mySettings;

		void UpdateDisplay();

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