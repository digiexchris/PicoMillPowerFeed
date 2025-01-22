#pragma once

#include "Event.hpp"
#include "StepperState.hpp"
#include <cstdint>
#include <memory>

enum class MachineState : uint8_t
{
	LEFT = 1,
	RIGHT = 2,
	RAPID = 4
};

enum class DeviceState : uint8_t
{
	LEFT_HIGH,
	LEFT_LOW,
	RIGHT_HIGH,
	RIGHT_LOW,
	RAPID_HIGH,
	RAPID_LOW,
	NORMAL_SPEED_CHANGE,
	RAPID_SPEED_CHANGE
};

struct StateChange
{
	DeviceState type;
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

class Machine
{
public:
	Machine(std::shared_ptr<Stepper::StepperState> aStepperState, uint32_t aNormalSpeed, uint32_t aRapidSpeed) : myStepperState(aStepperState), myNormalSpeed(aNormalSpeed), myRapidSpeed(aRapidSpeed){};

	void OnValueChange(std::shared_ptr<StateChange> anStateChange);

private:
	std::shared_ptr<Stepper::StepperState> myStepperState;
	uint32_t myNormalSpeed;
	uint32_t myRapidSpeed;
	uint8_t myState = 0;

	void SetState(MachineState state)
	{
		myState |= static_cast<uint8_t>(state);
	}

	void ClearState(MachineState state)
	{
		myState &= ~static_cast<uint8_t>(state);
	}

	bool IsStateSet(MachineState state) const
	{
		return (myState & static_cast<uint8_t>(state)) != 0;
	}
};