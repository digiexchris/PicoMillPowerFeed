#include "../src/MachineState.hpp"
#include "../src/config.hpp"
#include "TestCommon.hpp"
#include "TestDisplay.hpp"
#include "TestStepper.hpp"
#include <gtest/gtest.h>
#include <memory>

using ::testing::Return;

using namespace PicoMill;

class MachineStateTest : public ::testing::Test
{
protected:
	void SetUp() override
	{
		display = std::make_shared<PicoMill::TestDisplay>();
		stepper = std::make_shared<PicoMill::Drivers::TestStepper>();
		time = std::make_shared<TestTime>();
		stepperState = std::make_shared<PicoMill::TestStepperState>(stepper, time);
		state = std::make_unique<Machine>(display, stepperState, 100, 200);
	}

	std::shared_ptr<PicoMill::TestDisplay> display;
	std::shared_ptr<PicoMill::Drivers::TestStepper> stepper;
	std::shared_ptr<TestTime> time;
	std::shared_ptr<PicoMill::TestStepperState> stepperState;
	std::shared_ptr<Machine> state;
};

bool compareCommands(std::shared_ptr<PicoMill::Command> cmd)
{
	auto startCmd = std::static_pointer_cast<PicoMill::Start>(cmd);
	return startCmd->direction == leftDir &&
		   startCmd->speed == 100;
}

TEST_F(MachineStateTest, Left_Normal_Speed)
{
	// Set up expectation before executing the action
	EXPECT_CALL(*stepperState, ProcessCommand(::testing::Truly([](std::shared_ptr<PicoMill::Command> cmd)
															   {
		auto startCmd = std::static_pointer_cast<PicoMill::Start>(cmd);
		return startCmd->direction == leftDir && startCmd->speed == 100; })))
		.Times(1);

	// Execute the action after setting up expectations
	auto stateChange = std::make_shared<BoolStateChange>(DeviceState::LEFT_HIGH);
	state->OnValueChange(stateChange);
}

TEST_F(MachineStateTest, Right_Normal_Speed)
{

	// Set up expectation before executing the action
	EXPECT_CALL(*stepperState, ProcessCommand(::testing::Truly([](std::shared_ptr<PicoMill::Command> cmd)
															   {
		auto startCmd = std::static_pointer_cast<PicoMill::Start>(cmd);
		return startCmd->direction == rightDir && startCmd->speed == 100; })))
		.Times(1);

	// Execute the action after setting up expectations
	auto stateChange = std::make_shared<BoolStateChange>(DeviceState::RIGHT_HIGH);
	state->OnValueChange(stateChange);
}
TEST_F(MachineStateTest, Left_Rapid_Speed)
{
	// Set rapid speed first
	auto rapidStateChange = std::make_shared<BoolStateChange>(DeviceState::RAPID_HIGH);
	state->OnValueChange(rapidStateChange);

	// Set up expectation for left movement at rapid speed
	EXPECT_CALL(*stepperState, ProcessCommand(::testing::Truly([](std::shared_ptr<PicoMill::Command> cmd)
															   {
		auto startCmd = std::static_pointer_cast<PicoMill::Start>(cmd);
		return startCmd->direction == leftDir && startCmd->speed == 200; })))
		.Times(1);

	auto stateChange = std::make_shared<BoolStateChange>(DeviceState::LEFT_HIGH);
	state->OnValueChange(stateChange);
}

TEST_F(MachineStateTest, Right_Rapid_Speed)
{

	// Set rapid speed first
	auto rapidStateChange = std::make_shared<BoolStateChange>(DeviceState::RAPID_HIGH);
	state->OnValueChange(rapidStateChange);

	// Set up expectation for right movement at rapid speed
	EXPECT_CALL(*stepperState, ProcessCommand(::testing::Truly([](std::shared_ptr<PicoMill::Command> cmd)
															   {
		auto startCmd = std::static_pointer_cast<PicoMill::Start>(cmd);
		return startCmd->direction == rightDir && startCmd->speed == 200; })))
		.Times(1);

	auto stateChange = std::make_shared<BoolStateChange>(DeviceState::RIGHT_HIGH);
	state->OnValueChange(stateChange);
}

TEST_F(MachineStateTest, Right_Normal_Then_Rapid_Then_Normal_Then_Stop)
{

	// Set up expectation for right movement at normal speed
	EXPECT_CALL(*stepperState, ProcessCommand(::testing::Truly([](std::shared_ptr<PicoMill::Command> cmd)
															   {
		auto startCmd = std::static_pointer_cast<PicoMill::Start>(cmd);
			return startCmd->direction == rightDir && startCmd->speed == 100; })))
		.Times(1);
	auto stateChange = std::make_shared<BoolStateChange>(DeviceState::RIGHT_HIGH);
	state->OnValueChange(stateChange);

	EXPECT_CALL(*stepperState, ProcessCommand(::testing::Truly([](std::shared_ptr<PicoMill::Command> cmd)
															   {
		auto speedCmd = std::static_pointer_cast<PicoMill::ChangeSpeed>(cmd);
		return speedCmd->speed == 200; })))
		.Times(1);
	// Set up expectation for right movement at rapid speed
	auto rapidStateChange = std::make_shared<BoolStateChange>(DeviceState::RAPID_HIGH);
	state->OnValueChange(rapidStateChange);

	EXPECT_CALL(*stepperState, ProcessCommand(::testing::Truly([](std::shared_ptr<PicoMill::Command> cmd)
															   {
		auto speedCmd = std::static_pointer_cast<PicoMill::ChangeSpeed>(cmd);
		return speedCmd->speed == 100; })))
		.Times(1);
	stateChange = std::make_shared<BoolStateChange>(DeviceState::RAPID_LOW);
	state->OnValueChange(stateChange);

	// Set up expectation for right movement at normal speed
	EXPECT_CALL(*stepperState, ProcessCommand(::testing::Truly([](std::shared_ptr<PicoMill::Command> cmd)
															   {
		auto stopCmd = std::static_pointer_cast<PicoMill::Stop>(cmd);
		return stopCmd->type == PicoMill::Command::Type::STOP ; })))
		.Times(1);
	stateChange = std::make_shared<BoolStateChange>(DeviceState::RIGHT_LOW);
	state->OnValueChange(stateChange);
}

TEST_F(MachineStateTest, Right_Normal_Then_ChangeSpeed)
{
	// Start right movement at normal speed
	EXPECT_CALL(*stepperState, ProcessCommand(::testing::Truly([](std::shared_ptr<PicoMill::Command> cmd)
															   {
		auto startCmd = std::static_pointer_cast<PicoMill::Start>(cmd);
		return startCmd->direction == rightDir && startCmd->speed == 100; })))
		.Times(1);
	auto stateChange = std::make_shared<BoolStateChange>(DeviceState::RIGHT_HIGH);
	state->OnValueChange(stateChange);

	// Change normal speed while moving
	EXPECT_CALL(*stepperState, ProcessCommand(::testing::Truly([](std::shared_ptr<PicoMill::Command> cmd)
															   {
		auto speedCmd = std::static_pointer_cast<PicoMill::ChangeSpeed>(cmd);
		return speedCmd->speed == 100 + ENCODER_COUNTS_TO_STEPS_PER_SECOND; })))
		.Times(1);
	auto speedChange = std::make_shared<Int8StateChange>(DeviceState::ENCODER_CHANGED, 1);
	state->OnValueChange(speedChange);
}

TEST_F(MachineStateTest, Right_Normal_Then_Rapid_Then_Change_rapid_speed_Changes_Rapid_Speed)
{
	// Start right movement at normal speed
	EXPECT_CALL(*stepperState, ProcessCommand(::testing::Truly([](std::shared_ptr<PicoMill::Command> cmd)
															   {
		auto startCmd = std::static_pointer_cast<PicoMill::Start>(cmd);
		return startCmd->direction == rightDir && startCmd->speed == 100; })))
		.Times(1);
	auto stateChange = std::make_shared<BoolStateChange>(DeviceState::RIGHT_HIGH);
	state->OnValueChange(stateChange);

	// Switch to rapid speed
	EXPECT_CALL(*stepperState, ProcessCommand(::testing::Truly([](std::shared_ptr<PicoMill::Command> cmd)
															   {
		auto speedCmd = std::static_pointer_cast<PicoMill::ChangeSpeed>(cmd);
		return speedCmd->speed == 200; })))
		.Times(1);
	auto rapidStateChange = std::make_shared<BoolStateChange>(DeviceState::RAPID_HIGH);
	state->OnValueChange(rapidStateChange);

	// Change rapid speed while in rapid mode
	EXPECT_CALL(*stepperState, ProcessCommand(::testing::Truly([](std::shared_ptr<PicoMill::Command> cmd)
															   {
		auto speedCmd = std::static_pointer_cast<PicoMill::ChangeSpeed>(cmd);
		return speedCmd->speed == 200 + ENCODER_COUNTS_TO_STEPS_PER_SECOND; })))
		.Times(1);
	auto encoderStateChange = std::make_shared<Int8StateChange>(DeviceState::ENCODER_CHANGED, 1);
	state->OnValueChange(encoderStateChange);

	// Change encoder while in rapid mode - should not affect current speed
	auto speedChange = std::make_shared<Int8StateChange>(DeviceState::ENCODER_CHANGED, 1);
	state->OnValueChange(speedChange);

	// Switch to normal speed
	EXPECT_CALL(*stepperState, ProcessCommand(::testing::Truly([](std::shared_ptr<PicoMill::Command> cmd)
															   {
		auto speedCmd = std::static_pointer_cast<PicoMill::ChangeSpeed>(cmd);
		return speedCmd->speed == 150; })))
		.Times(1);
	auto normalStateChange = std::make_shared<BoolStateChange>(DeviceState::RAPID_LOW);
	state->OnValueChange(normalStateChange);
}

// TODO test cases needed: negative speed change doesn't go below zero. If it goes to zero, set it to the minimum jerk speed
// TODO need acceleration test cases
// TODO need to add display calls to all test cases

// Probably not needed because encoder changes the active speed.
// TEST_F(MachineStateTest, Right_Normal_Then_Change_rapid_speed_Maintains_normal_Speed)
// {
// 	// Start right movement at normal speed
// 	EXPECT_CALL(*stepperState, ProcessCommand(::testing::Truly([](std::shared_ptr<PicoMill::Command> cmd)
// 															   {
// 		auto startCmd = std::static_pointer_cast<PicoMill::Start>(cmd);
// 		return startCmd->direction == rightDir && startCmd->speed == 100; })))
// 		.Times(1);
// 	auto stateChange = std::make_shared<BoolStateChange>(DeviceState::RIGHT_HIGH);
// 	state->OnValueChange(stateChange);

// 	// Change rapid speed while in normal mode - should not affect current speed
// 	auto speedChange = std::make_shared < std::make_shared<Int8StateChange>(DeviceState::ENCODER_CHANGED, 1);
// 	state->OnValueChange(speedChange);

// 	// Switch to rapid speed
// 	EXPECT_CALL(*stepperState, ProcessCommand(::testing::Truly([](std::shared_ptr<PicoMill::Command> cmd)
// 															   {
// 		auto speedCmd = std::static_pointer_cast<PicoMill::ChangeSpeed>(cmd);
// 		return speedCmd->speed == 250; })))
// 		.Times(1);
// 	auto rapidStateChange = std::make_shared<BoolStateChange>(DeviceState::RAPID_HIGH);
// 	state->OnValueChange(rapidStateChange);
// }