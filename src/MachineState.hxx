#pragma once

#include "Display.hxx"
#include "Event.hxx"
#include "StepperState.hxx"
#include "config.hxx"
// #include <memory>
#include <stdint.h>

namespace PicoMill
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

	class Machine
	{
	public:
		Machine(Display &aDisplay, StepperState &aStepperState, uint32_t aNormalSpeed = ACCELERATION_JERK, uint32_t aRapidSpeed = ACCELERATION_JERK * 2);
		Machine(const Machine &) = delete;
		Machine &operator=(const Machine &) = delete;

		void OnValueChange(StateChange &anStateChange);

		bool IsStateSet(MachineState state) const
		{
			return (myState & static_cast<uint8_t>(state)) != 0;
		}

		Display &GetDisplay() const { return myDisplay; }

	private:
		Display &myDisplay;
		StepperState &myStepperState;
		uint32_t myNormalSpeed = ACCELERATION_JERK;
		uint32_t myRapidSpeed = ACCELERATION_JERK;
		uint32_t myAcceleration;
		uint8_t myState = 0;
		Units myUnits = Units::Millimeter;

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

} // namespace PicoMill