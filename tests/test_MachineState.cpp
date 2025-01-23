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

TEST(MachineState, Right_Normal_Then_Rapid_Then_Normal_Then_Stop)
{
	std::shared_ptr<TestStepper> stepper = std::make_shared<TestStepper>();
	std::shared_ptr<TestTime> time = std::make_shared<TestTime>();
	std::shared_ptr<Stepper::TestStepperState> stepperState = std::make_shared<Stepper::TestStepperState>(stepper, time);
	Machine state(stepperState, 100, 200);

	// Set up expectation for right movement at normal speed
	EXPECT_CALL(*stepperState, ProcessCommand(::testing::Truly([](std::shared_ptr<Stepper::Command> cmd)
															   {
		auto startCmd = std::static_pointer_cast<Stepper::Start>(cmd);
			return startCmd->direction == rightDir && startCmd->speed == 100; })))
		.Times(1);
	auto stateChange = std::make_shared<BoolStateChange>(DeviceState::RIGHT_HIGH);
	state.OnValueChange(stateChange);

	EXPECT_CALL(*stepperState, ProcessCommand(::testing::Truly([](std::shared_ptr<Stepper::Command> cmd)
															   {
		auto speedCmd = std::static_pointer_cast<Stepper::ChangeSpeed>(cmd);
		return speedCmd->speed == 200; })))
		.Times(1);
	// Set up expectation for right movement at rapid speed
	auto rapidStateChange = std::make_shared<BoolStateChange>(DeviceState::RAPID_HIGH);
	state.OnValueChange(rapidStateChange);

	EXPECT_CALL(*stepperState, ProcessCommand(::testing::Truly([](std::shared_ptr<Stepper::Command> cmd)
															   {
		auto speedCmd = std::static_pointer_cast<Stepper::ChangeSpeed>(cmd);
		return speedCmd->speed == 100; })))
		.Times(1);
	stateChange = std::make_shared<BoolStateChange>(DeviceState::RAPID_LOW);
	state.OnValueChange(stateChange);

	// Set up expectation for right movement at normal speed
	EXPECT_CALL(*stepperState, ProcessCommand(::testing::Truly([](std::shared_ptr<Stepper::Command> cmd)
															   {
		auto stopCmd = std::static_pointer_cast<Stepper::Stop>(cmd);
		return stopCmd->type == Stepper::Command::Type::STOP ; })))
		.Times(1);
	stateChange = std::make_shared<BoolStateChange>(DeviceState::RIGHT_LOW);
	state.OnValueChange(stateChange);
}

TEST(MachineState, Right_Normal_Then_ChangeSpeed)
{
	std::shared_ptr<TestStepper> stepper = std::make_shared<TestStepper>();
	std::shared_ptr<TestTime> time = std::make_shared<TestTime>();
	std::shared_ptr<Stepper::TestStepperState> stepperState = std::make_shared<Stepper::TestStepperState>(stepper, time);
	Machine state(stepperState, 100, 200);

	// Start right movement at normal speed
	EXPECT_CALL(*stepperState, ProcessCommand(::testing::Truly([](std::shared_ptr<Stepper::Command> cmd)
															   {
		auto startCmd = std::static_pointer_cast<Stepper::Start>(cmd);
		return startCmd->direction == rightDir && startCmd->speed == 100; })))
		.Times(1);
	auto stateChange = std::make_shared<BoolStateChange>(DeviceState::RIGHT_HIGH);
	state.OnValueChange(stateChange);

	// Change normal speed while moving
	EXPECT_CALL(*stepperState, ProcessCommand(::testing::Truly([](std::shared_ptr<Stepper::Command> cmd)
															   {
		auto speedCmd = std::static_pointer_cast<Stepper::ChangeSpeed>(cmd);
		return speedCmd->speed == 150; })))
		.Times(1);
	auto speedChange = std::make_shared<UInt32StateChange>(DeviceState::NORMAL_SPEED_CHANGE, 150);
	state.OnValueChange(speedChange);
}

TEST(MachineState, Right_Normal_Then_Rapid_Then_Change_normal_speed_Maintains_Rapid_Speed)
{
	std::shared_ptr<TestStepper> stepper = std::make_shared<TestStepper>();
	std::shared_ptr<TestTime> time = std::make_shared<TestTime>();
	std::shared_ptr<Stepper::TestStepperState> stepperState = std::make_shared<Stepper::TestStepperState>(stepper, time);
	Machine state(stepperState, 100, 200);

	// Start right movement at normal speed
	EXPECT_CALL(*stepperState, ProcessCommand(::testing::Truly([](std::shared_ptr<Stepper::Command> cmd)
															   {
		auto startCmd = std::static_pointer_cast<Stepper::Start>(cmd);
		return startCmd->direction == rightDir && startCmd->speed == 100; })))
		.Times(1);
	auto stateChange = std::make_shared<BoolStateChange>(DeviceState::RIGHT_HIGH);
	state.OnValueChange(stateChange);

	// Switch to rapid speed
	EXPECT_CALL(*stepperState, ProcessCommand(::testing::Truly([](std::shared_ptr<Stepper::Command> cmd)
															   {
		auto speedCmd = std::static_pointer_cast<Stepper::ChangeSpeed>(cmd);
		return speedCmd->speed == 200; })))
		.Times(1);
	auto rapidStateChange = std::make_shared<BoolStateChange>(DeviceState::RAPID_HIGH);
	state.OnValueChange(rapidStateChange);

	// Change normal speed while in rapid mode - should not affect current speed
	auto speedChange = std::make_shared<UInt32StateChange>(DeviceState::NORMAL_SPEED_CHANGE, 150);
	state.OnValueChange(speedChange);

	// Switch to normal speed
	EXPECT_CALL(*stepperState, ProcessCommand(::testing::Truly([](std::shared_ptr<Stepper::Command> cmd)
															   {
		auto speedCmd = std::static_pointer_cast<Stepper::ChangeSpeed>(cmd);
		return speedCmd->speed == 150; })))
		.Times(1);
	auto normalStateChange = std::make_shared<BoolStateChange>(DeviceState::RAPID_LOW);
	state.OnValueChange(normalStateChange);
}

TEST(MachineState, Right_Normal_Then_Change_rapid_speed_Maintains_normal_Speed)
{
	std::shared_ptr<TestStepper> stepper = std::make_shared<TestStepper>();
	std::shared_ptr<TestTime> time = std::make_shared<TestTime>();
	std::shared_ptr<Stepper::TestStepperState> stepperState = std::make_shared<Stepper::TestStepperState>(stepper, time);
	Machine state(stepperState, 100, 200);

	// Start right movement at normal speed
	EXPECT_CALL(*stepperState, ProcessCommand(::testing::Truly([](std::shared_ptr<Stepper::Command> cmd)
															   {
		auto startCmd = std::static_pointer_cast<Stepper::Start>(cmd);
		return startCmd->direction == rightDir && startCmd->speed == 100; })))
		.Times(1);
	auto stateChange = std::make_shared<BoolStateChange>(DeviceState::RIGHT_HIGH);
	state.OnValueChange(stateChange);

	// Change rapid speed while in normal mode - should not affect current speed
	auto speedChange = std::make_shared<UInt32StateChange>(DeviceState::RAPID_SPEED_CHANGE, 250);
	state.OnValueChange(speedChange);

	// Switch to rapid speed
	EXPECT_CALL(*stepperState, ProcessCommand(::testing::Truly([](std::shared_ptr<Stepper::Command> cmd)
															   {
		auto speedCmd = std::static_pointer_cast<Stepper::ChangeSpeed>(cmd);
		return speedCmd->speed == 250; })))
		.Times(1);
	auto rapidStateChange = std::make_shared<BoolStateChange>(DeviceState::RAPID_HIGH);
	state.OnValueChange(rapidStateChange);
}