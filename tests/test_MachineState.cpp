#include "../src/MachineState.hpp"
#include "../src/config.hpp"
#include "TestCommon.hpp"
#include "TestStepper.hpp"
#include <gtest/gtest.h>
#include <memory>

using ::testing::Return;

TEST(MachineState, INIT)
{
	std::shared_ptr<TestStepper> stepper = std::make_shared<TestStepper>();
	std::shared_ptr<TestTime> time = std::make_shared<TestTime>();
	std::shared_ptr<Stepper::TestStepperState> stepperState = std::make_shared<Stepper::TestStepperState>(stepper, time);
	Machine state(stepperState, 100, 200);
}

bool compareCommands(std::shared_ptr<Stepper::Command> cmd)
{
	auto startCmd = std::static_pointer_cast<Stepper::Start>(cmd);
	return startCmd->direction == leftDir &&
		   startCmd->speed == 100;
}

TEST(MachineState, Left_Normal_Speed)
{
	std::shared_ptr<TestStepper> stepper = std::make_shared<TestStepper>();
	std::shared_ptr<TestTime> time = std::make_shared<TestTime>();
	std::shared_ptr<Stepper::TestStepperState> stepperState = std::make_shared<Stepper::TestStepperState>(stepper, time);
	Machine state(stepperState, 100, 200);

	// Set up expectation before executing the action
	EXPECT_CALL(*stepperState, ProcessCommand(::testing::Truly([](std::shared_ptr<Stepper::Command> cmd)
															   {
		auto startCmd = std::static_pointer_cast<Stepper::Start>(cmd);
		return startCmd->direction == leftDir && startCmd->speed == 100; })))
		.Times(1);

	// Execute the action after setting up expectations
	auto stateChange = std::make_shared<BoolStateChange>(DeviceState::LEFT_HIGH);
	state.OnValueChange(stateChange);
}

TEST(MachineState, Right_Normal_Speed)
{
	std::shared_ptr<TestStepper> stepper = std::make_shared<TestStepper>();
	std::shared_ptr<TestTime> time = std::make_shared<TestTime>();
	std::shared_ptr<Stepper::TestStepperState> stepperState = std::make_shared<Stepper::TestStepperState>(stepper, time);
	Machine state(stepperState, 100, 200);

	// Set up expectation before executing the action
	EXPECT_CALL(*stepperState, ProcessCommand(::testing::Truly([](std::shared_ptr<Stepper::Command> cmd)
															   {
		auto startCmd = std::static_pointer_cast<Stepper::Start>(cmd);
		return startCmd->direction == rightDir && startCmd->speed == 100; })))
		.Times(1);

	// Execute the action after setting up expectations
	auto stateChange = std::make_shared<BoolStateChange>(DeviceState::RIGHT_HIGH);
	state.OnValueChange(stateChange);
}
TEST(MachineState, Left_Rapid_Speed)
{
	std::shared_ptr<TestStepper> stepper = std::make_shared<TestStepper>();
	std::shared_ptr<TestTime> time = std::make_shared<TestTime>();
	std::shared_ptr<Stepper::TestStepperState> stepperState = std::make_shared<Stepper::TestStepperState>(stepper, time);
	Machine state(stepperState, 100, 200);

	// Set rapid speed first
	auto rapidStateChange = std::make_shared<BoolStateChange>(DeviceState::RAPID_HIGH);
	state.OnValueChange(rapidStateChange);

	// Set up expectation for left movement at rapid speed
	EXPECT_CALL(*stepperState, ProcessCommand(::testing::Truly([](std::shared_ptr<Stepper::Command> cmd)
															   {
		auto startCmd = std::static_pointer_cast<Stepper::Start>(cmd);
		return startCmd->direction == leftDir && startCmd->speed == 200; })))
		.Times(1);

	auto stateChange = std::make_shared<BoolStateChange>(DeviceState::LEFT_HIGH);
	state.OnValueChange(stateChange);
}

TEST(MachineState, Right_Rapid_Speed)
{
	std::shared_ptr<TestStepper> stepper = std::make_shared<TestStepper>();
	std::shared_ptr<TestTime> time = std::make_shared<TestTime>();
	std::shared_ptr<Stepper::TestStepperState> stepperState = std::make_shared<Stepper::TestStepperState>(stepper, time);
	Machine state(stepperState, 100, 200);

	// Set rapid speed first
	auto rapidStateChange = std::make_shared<BoolStateChange>(DeviceState::RAPID_HIGH);
	state.OnValueChange(rapidStateChange);

	// Set up expectation for right movement at rapid speed
	EXPECT_CALL(*stepperState, ProcessCommand(::testing::Truly([](std::shared_ptr<Stepper::Command> cmd)
															   {
		auto startCmd = std::static_pointer_cast<Stepper::Start>(cmd);
		return startCmd->direction == rightDir && startCmd->speed == 200; })))
		.Times(1);

	auto stateChange = std::make_shared<BoolStateChange>(DeviceState::RIGHT_HIGH);
	state.OnValueChange(stateChange);
}