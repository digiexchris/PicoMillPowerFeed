#include "../src/MachineState.hxx"
#include "../src/Settings.hxx"
#include "TestCommon.hpp"
#include "TestDisplay.hpp"
#include "TestStepper.hpp"
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <memory>

using namespace PowerFeed;

class MachineStateTest : public ::testing::Test
{
protected:
	void SetUp() override
	{
		mySettings = std::make_shared<SettingsManager>();
		MOVE_LEFT_DIRECTION = mySettings->Get()->mechanical.moveLeftDirection;
		MOVE_RIGHT_DIRECTION = mySettings->Get()->mechanical.moveRightDirection;
		ENCODER_COUNTS_TO_STEPS_PER_SECOND = mySettings->Get()->controls.encoderCountsToStepsPerSecond;
		ACCELERATION_JERK = mySettings->Get()->mechanical.accelerationJerk;
		display = std::make_shared<MockDisplay>();
		stepper = std::make_shared<::Drivers::TestStepper>();
		time = std::make_shared<TestTime>();
		stepperState = std::make_shared<::TestStepperState>(mySettings.get(), stepper.get());
		state = std::make_unique<Machine<Drivers::TestStepper>>(mySettings.get(), display.get(), stepperState.get(), 100, 200);
	}

	std::shared_ptr<MockDisplay> display;
	std::shared_ptr<::Drivers::TestStepper> stepper;
	std::shared_ptr<TestTime> time;
	std::shared_ptr<::TestStepperState> stepperState;
	std::shared_ptr<Machine<Drivers::TestStepper>> state;
	std::shared_ptr<SettingsManager> mySettings;
	bool MOVE_LEFT_DIRECTION;
	bool MOVE_RIGHT_DIRECTION;
	uint32_t ENCODER_COUNTS_TO_STEPS_PER_SECOND;
	uint32_t ACCELERATION_JERK;
};

TEST_F(MachineStateTest, Left_Normal_Speed)
{
	// Set up expectations before executing the action
	EXPECT_CALL(*display, ClearBuffer()).Times(1);
	EXPECT_CALL(*display, DrawSpeed(100)).Times(1);
	EXPECT_CALL(*display, DrawMovingLeft()).Times(1);
	EXPECT_CALL(*display, Refresh()).Times(1);
	EXPECT_CALL(*stepperState, ProcessCommand(::testing::Truly([this](const ::Command &cmd)
															   {
		// Verify command type first
		if (cmd.type != ::Command::Type::START) {
			printf("Expected START command, got: %d\n", static_cast<int>(cmd.type));
			return false;
		}
		
		// Cast to Start and verify properties
		auto startCmd = static_cast<const ::Start&>(cmd);
		bool result = startCmd.direction == MOVE_LEFT_DIRECTION && startCmd.speed == 100;
		
		if (!result) {
			printf("Command check failed: direction=%d (expected=%d), speed=%u (expected=100)\n", 
			       startCmd.direction, MOVE_LEFT_DIRECTION, startCmd.speed);
		}
		
		return result; })))
		.Times(1);

	// Execute the action
	BoolStateChange stateChange(DeviceState::LEFT_HIGH);
	state->OnValueChange(stateChange);
}

TEST_F(MachineStateTest, Right_Normal_Speed)
{

	// Set up expectation before executing the action
	EXPECT_CALL(*display, ClearBuffer()).Times(1);
	EXPECT_CALL(*display, DrawSpeed(100)).Times(1);
	EXPECT_CALL(*display, DrawMovingRight()).Times(1);
	EXPECT_CALL(*display, Refresh()).Times(1);
	EXPECT_CALL(*stepperState, ProcessCommand(::testing::Truly([this](const ::Command &cmd)
															   {
		auto startCmd = static_cast<const ::Start&>(cmd);
		return startCmd.direction == MOVE_RIGHT_DIRECTION && startCmd.speed == 100; })))
		.Times(1);

	// Execute the action after setting up expectations
	BoolStateChange stateChange(DeviceState::RIGHT_HIGH);
	state->OnValueChange(stateChange);
}
TEST_F(MachineStateTest, Left_Rapid_Speed)
{
	// Set rapid speed first
	EXPECT_CALL(*display, ClearBuffer()).Times(1);
	EXPECT_CALL(*display, DrawSpeed(200)).Times(1);
	EXPECT_CALL(*display, DrawStopped()).Times(1);
	EXPECT_CALL(*display, Refresh()).Times(1);
	BoolStateChange rapidStateChange(DeviceState::RAPID_HIGH);
	state->OnValueChange(rapidStateChange);

	// Set up expectation for left movement at rapid speed
	EXPECT_CALL(*display, ClearBuffer()).Times(1);
	EXPECT_CALL(*display, DrawSpeed(200)).Times(1);
	EXPECT_CALL(*display, DrawRapidLeft()).Times(1);
	EXPECT_CALL(*display, Refresh()).Times(1);
	EXPECT_CALL(*stepperState, ProcessCommand(::testing::Truly([this](const ::Command &cmd)
															   {
		auto startCmd = static_cast<const ::Start&>(cmd);
		return startCmd.direction == MOVE_LEFT_DIRECTION && startCmd.speed == 200; })))
		.Times(1);

	BoolStateChange stateChange(DeviceState::LEFT_HIGH);
	state->OnValueChange(stateChange);
}

TEST_F(MachineStateTest, Right_Rapid_Speed)
{

	// Set rapid speed first
	EXPECT_CALL(*display, ClearBuffer()).Times(1);
	EXPECT_CALL(*display, DrawSpeed(200)).Times(1);
	EXPECT_CALL(*display, DrawStopped()).Times(1);
	EXPECT_CALL(*display, Refresh()).Times(1);
	BoolStateChange rapidStateChange(DeviceState::RAPID_HIGH);
	state->OnValueChange(rapidStateChange);

	// Set up expectation for right movement at rapid speed
	EXPECT_CALL(*display, ClearBuffer()).Times(1);
	EXPECT_CALL(*display, DrawSpeed(200)).Times(1);
	EXPECT_CALL(*display, DrawRapidRight()).Times(1);
	EXPECT_CALL(*display, Refresh()).Times(1);
	EXPECT_CALL(*stepperState, ProcessCommand(::testing::Truly([this](const ::Command &cmd)
															   {
		auto startCmd = static_cast<const ::Start&>(cmd);
		return startCmd.direction == MOVE_RIGHT_DIRECTION && startCmd.speed == 200; })))
		.Times(1);

	BoolStateChange stateChange(DeviceState::RIGHT_HIGH);
	state->OnValueChange(stateChange);
}

TEST_F(MachineStateTest, Right_Normal_Then_Rapid_Then_Normal_Then_Stop)
{

	// Set up expectation for right movement at normal speed
	EXPECT_CALL(*display, ClearBuffer()).Times(1);
	EXPECT_CALL(*display, DrawSpeed(100)).Times(1);
	EXPECT_CALL(*display, DrawMovingRight()).Times(1);
	EXPECT_CALL(*display, Refresh()).Times(1);
	EXPECT_CALL(*stepperState, ProcessCommand(::testing::Truly([this](const ::Command &cmd)
															   {
		auto startCmd = static_cast<const ::Start&>(cmd);
			return startCmd.direction == MOVE_RIGHT_DIRECTION && startCmd.speed == 100; })))
		.Times(1);
	BoolStateChange stateChange(DeviceState::RIGHT_HIGH);
	state->OnValueChange(stateChange);

	EXPECT_CALL(*display, ClearBuffer()).Times(1);
	EXPECT_CALL(*display, DrawSpeed(200)).Times(1);
	EXPECT_CALL(*display, DrawRapidRight()).Times(1);
	EXPECT_CALL(*display, Refresh()).Times(1);
	EXPECT_CALL(*stepperState, ProcessCommand(::testing::Truly([](const ::Command &cmd)
															   {
		auto speedCmd = static_cast<const ::ChangeSpeed&>(cmd);
		return speedCmd.speed == 200; })))
		.Times(1);
	// Set up expectation for right movement at rapid speed
	BoolStateChange rapidStateChange(DeviceState::RAPID_HIGH);
	state->OnValueChange(rapidStateChange);

	EXPECT_CALL(*display, ClearBuffer()).Times(1);
	EXPECT_CALL(*display, DrawSpeed(100)).Times(1);
	EXPECT_CALL(*display, DrawMovingRight()).Times(1);
	EXPECT_CALL(*display, Refresh()).Times(1);
	EXPECT_CALL(*stepperState, ProcessCommand(::testing::Truly([](const ::Command &cmd)
															   {
		auto speedCmd = static_cast<const ::ChangeSpeed&>(cmd);
		return speedCmd.speed == 100; })))
		.Times(1);
	stateChange = BoolStateChange(DeviceState::RAPID_LOW);
	state->OnValueChange(stateChange);

	EXPECT_CALL(*display, ClearBuffer()).Times(1);
	EXPECT_CALL(*display, DrawSpeed(100)).Times(1);
	EXPECT_CALL(*display, DrawStopped()).Times(1);
	EXPECT_CALL(*display, Refresh()).Times(1);
	EXPECT_CALL(*stepperState, ProcessCommand(::testing::Truly([](const ::Command &cmd)
															   {
		auto stopCmd = static_cast<const ::Stop&>(cmd);
		return stopCmd.type == ::Command::Type::STOP; })))
		.Times(1);
	stateChange = BoolStateChange(DeviceState::RIGHT_LOW);
	state->OnValueChange(stateChange);
}

TEST_F(MachineStateTest, Right_Normal_Then_ChangeSpeed)
{
	// Start right movement at normal speed
	EXPECT_CALL(*display, ClearBuffer()).Times(1);
	EXPECT_CALL(*display, DrawSpeed(100)).Times(1);
	EXPECT_CALL(*display, DrawMovingRight()).Times(1);
	EXPECT_CALL(*display, Refresh()).Times(1);
	EXPECT_CALL(*stepperState, ProcessCommand(::testing::Truly([this](const ::Command &cmd)
															   {
		auto startCmd = static_cast<const ::Start&>(cmd);
		return startCmd.direction == MOVE_RIGHT_DIRECTION && startCmd.speed == 100; })))
		.Times(1);
	BoolStateChange stateChange(DeviceState::RIGHT_HIGH);
	state->OnValueChange(stateChange);

	// Change normal speed while moving
	EXPECT_CALL(*display, ClearBuffer()).Times(1);
	EXPECT_CALL(*display, DrawSpeed(100 + ENCODER_COUNTS_TO_STEPS_PER_SECOND)).Times(1);
	EXPECT_CALL(*display, DrawMovingRight()).Times(1);
	EXPECT_CALL(*display, Refresh()).Times(1);
	EXPECT_CALL(*stepperState, ProcessCommand(::testing::Truly([this](const ::Command &cmd)
															   {
		auto speedCmd = static_cast<const ::ChangeSpeed&>(cmd);
		return speedCmd.speed == 100 + ENCODER_COUNTS_TO_STEPS_PER_SECOND; })))
		.Times(1);
	ValueChange<int16_t> speedChange(DeviceState::ENCODER_CHANGED, 1);
	state->OnValueChange(speedChange);
}

TEST_F(MachineStateTest, Right_Normal_Then_Rapid_Then_Change_rapid_speed_Changes_Rapid_Speed)
{

	// Start right movement at normal speed
	EXPECT_CALL(*display, ClearBuffer())
		.Times(1);
	EXPECT_CALL(*display, DrawSpeed(100)).Times(1);
	EXPECT_CALL(*display, DrawMovingRight()).Times(1);
	EXPECT_CALL(*display, Refresh()).Times(1);
	EXPECT_CALL(*stepperState, ProcessCommand(::testing::Truly([this](const ::Command &cmd)
															   {
		auto startCmd = static_cast<const ::Start&>(cmd);
		return startCmd.direction == MOVE_RIGHT_DIRECTION && startCmd.speed == 100; })))
		.Times(1);
	BoolStateChange stateChange(DeviceState::RIGHT_HIGH);
	state->OnValueChange(stateChange);

	// Switch to rapid speed
	EXPECT_CALL(*display, ClearBuffer())
		.Times(1);
	EXPECT_CALL(*display, DrawSpeed(200)).Times(1);
	EXPECT_CALL(*display, DrawRapidRight()).Times(1);
	EXPECT_CALL(*display, Refresh()).Times(1);
	EXPECT_CALL(*stepperState, ProcessCommand(::testing::Truly([this](const ::Command &cmd)
															   {
		auto speedCmd = static_cast<const ::ChangeSpeed&>(cmd);
		return speedCmd.speed == 200; })))
		.Times(1);
	BoolStateChange rapidStateChange(DeviceState::RAPID_HIGH);
	state->OnValueChange(rapidStateChange);

	// Change rapid speed while in rapid mode
	EXPECT_CALL(*display, ClearBuffer()).Times(1);
	EXPECT_CALL(*display, DrawSpeed(200 + ENCODER_COUNTS_TO_STEPS_PER_SECOND)).Times(1);
	EXPECT_CALL(*display, DrawRapidRight()).Times(1);
	EXPECT_CALL(*display, Refresh()).Times(1);
	EXPECT_CALL(*stepperState, ProcessCommand(::testing::Truly([this](const ::Command &cmd)
															   {
		auto speedCmd = static_cast<const ::ChangeSpeed&>(cmd);
		return speedCmd.speed == 200 + ENCODER_COUNTS_TO_STEPS_PER_SECOND; })))
		.Times(1);
	ValueChange<int16_t> encoderStateChange(DeviceState::ENCODER_CHANGED, 1);
	state->OnValueChange(encoderStateChange);

	// Switch to normal speed
	EXPECT_CALL(*display, ClearBuffer()).Times(1);
	EXPECT_CALL(*display, DrawSpeed(100)).Times(1);
	EXPECT_CALL(*display, DrawMovingRight()).Times(1);
	EXPECT_CALL(*display, Refresh()).Times(1);
	EXPECT_CALL(*stepperState, ProcessCommand(::testing::Truly([this](const ::Command &cmd)
															   {
		auto speedCmd = static_cast<const ::ChangeSpeed&>(cmd);
		return speedCmd.speed == 100; })))
		.Times(1);
	BoolStateChange normalStateChange(DeviceState::RAPID_LOW);
	state->OnValueChange(normalStateChange);
}

TEST_F(MachineStateTest, NegativeSpeedChangeDoesntGoBelowMinimumJerk)
{
	// Start right movement at normal speed
	EXPECT_CALL(*display, ClearBuffer()).Times(1);
	EXPECT_CALL(*display, DrawSpeed(100)).Times(1);
	EXPECT_CALL(*display, DrawMovingRight()).Times(1);
	EXPECT_CALL(*display, Refresh()).Times(1);
	EXPECT_CALL(*stepperState, ProcessCommand(::testing::Truly([this](const ::Command &cmd)
															   {
		auto startCmd = static_cast<const ::Start&>(cmd);
		return startCmd.direction == MOVE_RIGHT_DIRECTION && startCmd.speed == 100; })))
		.Times(1);
	BoolStateChange stateChange(DeviceState::RIGHT_HIGH);
	state->OnValueChange(stateChange);

	// Decrease speed to minimum jerk
	EXPECT_CALL(*display, ClearBuffer()).Times(1);
	EXPECT_CALL(*display, DrawSpeed(ACCELERATION_JERK)).Times(1);
	EXPECT_CALL(*display, DrawMovingRight()).Times(1);
	EXPECT_CALL(*display, Refresh()).Times(1);
	EXPECT_CALL(*stepperState, ProcessCommand(::testing::Truly([this](const ::Command &cmd)
															   {
		auto speedCmd = static_cast<const ::ChangeSpeed&>(cmd);
		return speedCmd.speed == ACCELERATION_JERK; })))
		.Times(1);

	// Change encoder by large negative value that would put speed below zero
	ValueChange<int16_t> speedChange(DeviceState::ENCODER_CHANGED, -10);
	state->OnValueChange(speedChange);
}

TEST_F(MachineStateTest, NegativeRapidSpeedChangeDoesntGoBelowMinimumJerk)
{
	// Set rapid speed first
	EXPECT_CALL(*display, ClearBuffer()).Times(1);
	EXPECT_CALL(*display, DrawSpeed(200)).Times(1);
	EXPECT_CALL(*display, DrawStopped()).Times(1);
	EXPECT_CALL(*display, Refresh()).Times(1);
	BoolStateChange rapidStateChange(DeviceState::RAPID_HIGH);
	state->OnValueChange(rapidStateChange);

	// Start right movement at rapid speed
	EXPECT_CALL(*display, ClearBuffer()).Times(1);
	EXPECT_CALL(*display, DrawSpeed(200)).Times(1);
	EXPECT_CALL(*display, DrawRapidRight()).Times(1);
	EXPECT_CALL(*display, Refresh()).Times(1);
	EXPECT_CALL(*stepperState, ProcessCommand(::testing::Truly([this](const ::Command &cmd)
															   {
		auto startCmd = static_cast<const ::Start&>(cmd);
		return startCmd.direction == MOVE_RIGHT_DIRECTION && startCmd.speed == 200; })))
		.Times(1);
	BoolStateChange stateChange(DeviceState::RIGHT_HIGH);
	state->OnValueChange(stateChange);

	// Decrease rapid speed to minimum jerk
	EXPECT_CALL(*display, ClearBuffer()).Times(1);
	EXPECT_CALL(*display, DrawSpeed(ACCELERATION_JERK)).Times(1);
	EXPECT_CALL(*display, DrawRapidRight()).Times(1);
	EXPECT_CALL(*display, Refresh()).Times(1);
	EXPECT_CALL(*stepperState, ProcessCommand(::testing::Truly([this](const ::Command &cmd)
															   {
		auto speedCmd = static_cast<const ::ChangeSpeed&>(cmd);
		return speedCmd.speed == ACCELERATION_JERK; })))
		.Times(1);

	// Change encoder by large negative value that would put speed below minimum jerk
	ValueChange<int16_t> speedChange(DeviceState::ENCODER_CHANGED, -100);
	state->OnValueChange(speedChange);
}
