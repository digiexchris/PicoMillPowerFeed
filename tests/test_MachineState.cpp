#include "../src/MachineState.hxx"
#include "../src/config.hxx"
#include "TestCommon.hpp"
#include "TestDisplay.hpp"
#include "TestStepper.hpp"
#include <gtest/gtest.h>
#include <memory>

using namespace PowerFeed;

class MachineStateTest : public ::testing::Test
{
protected:
	void SetUp() override
	{
		display = std::make_shared<PowerFeed::TestDisplay>();
		stepper = std::make_shared<PowerFeed::Drivers::TestStepper>();
		time = std::make_shared<TestTime>();
		stepperState = std::make_shared<PowerFeed::TestStepperState>(stepper, time);
		state = std::make_unique<Machine>(display, stepperState, 100, 200);
	}

	std::shared_ptr<PowerFeed::TestDisplay> display;
	std::shared_ptr<PowerFeed::Drivers::TestStepper> stepper;
	std::shared_ptr<TestTime> time;
	std::shared_ptr<PowerFeed::TestStepperState> stepperState;
	std::shared_ptr<Machine> state;
};

bool compareCommands(std::shared_ptr<PowerFeed::Command> cmd)
{
	auto startCmd = std::static_pointer_cast<PowerFeed::Start>(cmd);
	return startCmd->direction == leftDir &&
		   startCmd->speed == 100;
}

TEST_F(MachineStateTest, Left_Normal_Speed)
{
	// Set up expectation before executing the action
	EXPECT_CALL(*display, Clear()).Times(1);
	EXPECT_CALL(*display, DrawSpeed(100)).Times(1);
	EXPECT_CALL(*display, DrawMovingLeft()).Times(1);
	EXPECT_CALL(*stepperState, ProcessCommand(::testing::Truly([](std::shared_ptr<PowerFeed::Command> cmd)
															   {
		auto startCmd = std::static_pointer_cast<PowerFeed::Start>(cmd);
		return startCmd->direction == leftDir && startCmd->speed == 100; })))
		.Times(1);

	// Execute the action after setting up expectations
	auto stateChange = std::make_shared<BoolStateChange>(DeviceState::LEFT_HIGH);
	state->OnValueChange(stateChange);
}

TEST_F(MachineStateTest, Right_Normal_Speed)
{

	// Set up expectation before executing the action
	EXPECT_CALL(*display, Clear()).Times(1);
	EXPECT_CALL(*display, DrawSpeed(100)).Times(1);
	EXPECT_CALL(*display, DrawMovingRight()).Times(1);
	EXPECT_CALL(*stepperState, ProcessCommand(::testing::Truly([](std::shared_ptr<PowerFeed::Command> cmd)
															   {
		auto startCmd = std::static_pointer_cast<PowerFeed::Start>(cmd);
		return startCmd->direction == rightDir && startCmd->speed == 100; })))
		.Times(1);

	// Execute the action after setting up expectations
	auto stateChange = std::make_shared<BoolStateChange>(DeviceState::RIGHT_HIGH);
	state->OnValueChange(stateChange);
}
TEST_F(MachineStateTest, Left_Rapid_Speed)
{
	// Set rapid speed first
	EXPECT_CALL(*display, Clear()).Times(1);
	EXPECT_CALL(*display, DrawSpeed(200)).Times(1);
	EXPECT_CALL(*display, DrawStopped()).Times(1);
	auto rapidStateChange = std::make_shared<BoolStateChange>(DeviceState::RAPID_HIGH);
	state->OnValueChange(rapidStateChange);

	// Set up expectation for left movement at rapid speed
	EXPECT_CALL(*display, Clear()).Times(1);
	EXPECT_CALL(*display, DrawSpeed(200)).Times(1);
	EXPECT_CALL(*display, DrawRapidLeft()).Times(1);
	EXPECT_CALL(*stepperState, ProcessCommand(::testing::Truly([](std::shared_ptr<PowerFeed::Command> cmd)
															   {
		auto startCmd = std::static_pointer_cast<PowerFeed::Start>(cmd);
		return startCmd->direction == leftDir && startCmd->speed == 200; })))
		.Times(1);

	auto stateChange = std::make_shared<BoolStateChange>(DeviceState::LEFT_HIGH);
	state->OnValueChange(stateChange);
}

TEST_F(MachineStateTest, Right_Rapid_Speed)
{

	// Set rapid speed first
	EXPECT_CALL(*display, Clear()).Times(1);
	EXPECT_CALL(*display, DrawSpeed(200)).Times(1);
	EXPECT_CALL(*display, DrawStopped()).Times(1);
	auto rapidStateChange = std::make_shared<BoolStateChange>(DeviceState::RAPID_HIGH);
	state->OnValueChange(rapidStateChange);

	// Set up expectation for right movement at rapid speed
	EXPECT_CALL(*display, Clear()).Times(1);
	EXPECT_CALL(*display, DrawSpeed(200)).Times(1);
	EXPECT_CALL(*display, DrawRapidRight()).Times(1);
	EXPECT_CALL(*stepperState, ProcessCommand(::testing::Truly([](std::shared_ptr<PowerFeed::Command> cmd)
															   {
		auto startCmd = std::static_pointer_cast<PowerFeed::Start>(cmd);
		return startCmd->direction == rightDir && startCmd->speed == 200; })))
		.Times(1);

	auto stateChange = std::make_shared<BoolStateChange>(DeviceState::RIGHT_HIGH);
	state->OnValueChange(stateChange);
}

TEST_F(MachineStateTest, Right_Normal_Then_Rapid_Then_Normal_Then_Stop)
{

	// Set up expectation for right movement at normal speed
	EXPECT_CALL(*display, Clear()).Times(1);
	EXPECT_CALL(*display, DrawSpeed(100)).Times(1);
	EXPECT_CALL(*display, DrawMovingRight()).Times(1);
	EXPECT_CALL(*stepperState, ProcessCommand(::testing::Truly([](std::shared_ptr<PowerFeed::Command> cmd)
															   {
		auto startCmd = std::static_pointer_cast<PowerFeed::Start>(cmd);
			return startCmd->direction == rightDir && startCmd->speed == 100; })))
		.Times(1);
	auto stateChange = std::make_shared<BoolStateChange>(DeviceState::RIGHT_HIGH);
	state->OnValueChange(stateChange);

	EXPECT_CALL(*display, Clear()).Times(1);
	EXPECT_CALL(*display, DrawSpeed(200)).Times(1);
	EXPECT_CALL(*display, DrawRapidRight()).Times(1);
	EXPECT_CALL(*stepperState, ProcessCommand(::testing::Truly([](std::shared_ptr<PowerFeed::Command> cmd)
															   {
		auto speedCmd = std::static_pointer_cast<PowerFeed::ChangeSpeed>(cmd);
		return speedCmd->speed == 200; })))
		.Times(1);
	// Set up expectation for right movement at rapid speed
	auto rapidStateChange = std::make_shared<BoolStateChange>(DeviceState::RAPID_HIGH);
	state->OnValueChange(rapidStateChange);

	EXPECT_CALL(*display, Clear()).Times(1);
	EXPECT_CALL(*display, DrawSpeed(100)).Times(1);
	EXPECT_CALL(*display, DrawMovingRight()).Times(1);
	EXPECT_CALL(*stepperState, ProcessCommand(::testing::Truly([](std::shared_ptr<PowerFeed::Command> cmd)
															   {
		auto speedCmd = std::static_pointer_cast<PowerFeed::ChangeSpeed>(cmd);
		return speedCmd->speed == 100; })))
		.Times(1);
	stateChange = std::make_shared<BoolStateChange>(DeviceState::RAPID_LOW);
	state->OnValueChange(stateChange);

	EXPECT_CALL(*display, Clear()).Times(1);
	EXPECT_CALL(*display, DrawSpeed(100)).Times(1);
	EXPECT_CALL(*display, DrawStopped()).Times(1);
	EXPECT_CALL(*stepperState, ProcessCommand(::testing::Truly([](std::shared_ptr<PowerFeed::Command> cmd)
															   {
		auto stopCmd = std::static_pointer_cast<PowerFeed::Stop>(cmd);
		return stopCmd->type == PowerFeed::Command::Type::STOP ; })))
		.Times(1);
	stateChange = std::make_shared<BoolStateChange>(DeviceState::RIGHT_LOW);
	state->OnValueChange(stateChange);
}

TEST_F(MachineStateTest, Right_Normal_Then_ChangeSpeed)
{
	// Start right movement at normal speed
	EXPECT_CALL(*display, Clear()).Times(1);
	EXPECT_CALL(*display, DrawSpeed(100)).Times(1);
	EXPECT_CALL(*display, DrawMovingRight()).Times(1);
	EXPECT_CALL(*stepperState, ProcessCommand(::testing::Truly([](std::shared_ptr<PowerFeed::Command> cmd)
															   {
		auto startCmd = std::static_pointer_cast<PowerFeed::Start>(cmd);
		return startCmd->direction == rightDir && startCmd->speed == 100; })))
		.Times(1);
	auto stateChange = std::make_shared<BoolStateChange>(DeviceState::RIGHT_HIGH);
	state->OnValueChange(stateChange);

	// Change normal speed while moving
	EXPECT_CALL(*display, Clear()).Times(1);
	EXPECT_CALL(*display, DrawSpeed(100 + ENCODER_COUNTS_TO_STEPS_PER_SECOND)).Times(1);
	EXPECT_CALL(*display, DrawMovingRight()).Times(1);
	EXPECT_CALL(*stepperState, ProcessCommand(::testing::Truly([](std::shared_ptr<PowerFeed::Command> cmd)
															   {
		auto speedCmd = std::static_pointer_cast<PowerFeed::ChangeSpeed>(cmd);
		return speedCmd->speed == 100 + ENCODER_COUNTS_TO_STEPS_PER_SECOND; })))
		.Times(1);
	auto speedChange = std::make_shared<Int8StateChange>(DeviceState::ENCODER_CHANGED, 1);
	state->OnValueChange(speedChange);
}

TEST_F(MachineStateTest, Right_Normal_Then_Rapid_Then_Change_rapid_speed_Changes_Rapid_Speed)
{

	// Start right movement at normal speed
	EXPECT_CALL(*display, Clear())
		.Times(1);
	EXPECT_CALL(*display, DrawSpeed(100)).Times(1);
	EXPECT_CALL(*display, DrawMovingRight()).Times(1);
	EXPECT_CALL(*stepperState, ProcessCommand(::testing::Truly([](std::shared_ptr<PowerFeed::Command> cmd)
															   {
		auto startCmd = std::static_pointer_cast<PowerFeed::Start>(cmd);
		return startCmd->direction == rightDir && startCmd->speed == 100; })))
		.Times(1);
	auto stateChange = std::make_shared<BoolStateChange>(DeviceState::RIGHT_HIGH);
	state->OnValueChange(stateChange);

	// Switch to rapid speed
	EXPECT_CALL(*display, Clear())
		.Times(1);
	EXPECT_CALL(*display, DrawSpeed(200)).Times(1);
	EXPECT_CALL(*display, DrawRapidRight()).Times(1);
	EXPECT_CALL(*stepperState, ProcessCommand(::testing::Truly([](std::shared_ptr<PowerFeed::Command> cmd)
															   {
		auto speedCmd = std::static_pointer_cast<PowerFeed::ChangeSpeed>(cmd);
		return speedCmd->speed == 200; })))
		.Times(1);
	auto rapidStateChange = std::make_shared<BoolStateChange>(DeviceState::RAPID_HIGH);
	state->OnValueChange(rapidStateChange);

	// Change rapid speed while in rapid mode
	EXPECT_CALL(*display, Clear()).Times(1);
	EXPECT_CALL(*display, DrawSpeed(200 + ENCODER_COUNTS_TO_STEPS_PER_SECOND)).Times(1);
	EXPECT_CALL(*display, DrawRapidRight()).Times(1);
	EXPECT_CALL(*stepperState, ProcessCommand(::testing::Truly([](std::shared_ptr<PowerFeed::Command> cmd)
															   {
		auto speedCmd = std::static_pointer_cast<PowerFeed::ChangeSpeed>(cmd);
		return speedCmd->speed == 200 + ENCODER_COUNTS_TO_STEPS_PER_SECOND; })))
		.Times(1);
	auto encoderStateChange = std::make_shared<Int8StateChange>(DeviceState::ENCODER_CHANGED, 1);
	state->OnValueChange(encoderStateChange);

	// Switch to normal speed
	EXPECT_CALL(*display, Clear()).Times(1);
	EXPECT_CALL(*display, DrawSpeed(100)).Times(1);
	EXPECT_CALL(*display, DrawMovingRight()).Times(1);
	EXPECT_CALL(*stepperState, ProcessCommand(::testing::Truly([](std::shared_ptr<PowerFeed::Command> cmd)
															   {
		auto speedCmd = std::static_pointer_cast<PowerFeed::ChangeSpeed>(cmd);
		return speedCmd->speed == 100; })))
		.Times(1);
	auto normalStateChange = std::make_shared<BoolStateChange>(DeviceState::RAPID_LOW);
	state->OnValueChange(normalStateChange);
}

TEST_F(MachineStateTest, NegativeSpeedChangeDoesntGoBelowMinimumJerk)
{
	// Start right movement at normal speed
	EXPECT_CALL(*display, Clear()).Times(1);
	EXPECT_CALL(*display, DrawSpeed(100)).Times(1);
	EXPECT_CALL(*display, DrawMovingRight()).Times(1);
	EXPECT_CALL(*stepperState, ProcessCommand(::testing::Truly([](std::shared_ptr<PowerFeed::Command> cmd)
															   {
		auto startCmd = std::static_pointer_cast<PowerFeed::Start>(cmd);
		return startCmd->direction == rightDir && startCmd->speed == 100; })))
		.Times(1);
	auto stateChange = std::make_shared<BoolStateChange>(DeviceState::RIGHT_HIGH);
	state->OnValueChange(stateChange);

	// Decrease speed to minimum jerk
	EXPECT_CALL(*display, Clear()).Times(1);
	EXPECT_CALL(*display, DrawSpeed(ACCELERATION_JERK)).Times(1);
	EXPECT_CALL(*display, DrawMovingRight()).Times(1);
	EXPECT_CALL(*stepperState, ProcessCommand(::testing::Truly([](std::shared_ptr<PowerFeed::Command> cmd)
															   {
		auto speedCmd = std::static_pointer_cast<PowerFeed::ChangeSpeed>(cmd);
		return speedCmd->speed == ACCELERATION_JERK; })))
		.Times(1);

	// Change encoder by large negative value that would put speed below zero
	auto speedChange = std::make_shared<Int8StateChange>(DeviceState::ENCODER_CHANGED, -1);
	state->OnValueChange(speedChange);
}

TEST_F(MachineStateTest, NegativeRapidSpeedChangeDoesntGoBelowMinimumJerk)
{
	// Set rapid speed first
	EXPECT_CALL(*display, Clear()).Times(1);
	EXPECT_CALL(*display, DrawSpeed(200)).Times(1);
	EXPECT_CALL(*display, DrawStopped()).Times(1);
	auto rapidStateChange = std::make_shared<BoolStateChange>(DeviceState::RAPID_HIGH);
	state->OnValueChange(rapidStateChange);

	// Start right movement at rapid speed
	EXPECT_CALL(*display, Clear()).Times(1);
	EXPECT_CALL(*display, DrawSpeed(200)).Times(1);
	EXPECT_CALL(*display, DrawRapidRight()).Times(1);
	EXPECT_CALL(*stepperState, ProcessCommand(::testing::Truly([](std::shared_ptr<PowerFeed::Command> cmd)
															   {
		auto startCmd = std::static_pointer_cast<PowerFeed::Start>(cmd);
		return startCmd->direction == rightDir && startCmd->speed == 200; })))
		.Times(1);
	auto stateChange = std::make_shared<BoolStateChange>(DeviceState::RIGHT_HIGH);
	state->OnValueChange(stateChange);

	// Decrease rapid speed to minimum jerk
	EXPECT_CALL(*display, Clear()).Times(1);
	EXPECT_CALL(*display, DrawSpeed(ACCELERATION_JERK)).Times(1);
	EXPECT_CALL(*display, DrawRapidRight()).Times(1);
	EXPECT_CALL(*stepperState, ProcessCommand(::testing::Truly([](std::shared_ptr<PowerFeed::Command> cmd)
															   {
		auto speedCmd = std::static_pointer_cast<PowerFeed::ChangeSpeed>(cmd);
		return speedCmd->speed == ACCELERATION_JERK; })))
		.Times(1);

	// Change encoder by large negative value that would put speed below minimum jerk
	auto speedChange = std::make_shared<Int8StateChange>(DeviceState::ENCODER_CHANGED, -100);
	state->OnValueChange(speedChange);
}
