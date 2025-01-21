#include "../src/State.hpp"
#include "TestStepper.hpp"
#include <gtest/gtest.h>

using ::testing::Return;

TEST(State, Start)
{
	std::shared_ptr<TestStepper> stepper = std::make_shared<TestStepper>();
	State state(stepper);
	EXPECT_CALL(*stepper, GetCurrentSpeed()).WillOnce(Return(0));
	EXPECT_CALL(*stepper, GetDirection()).WillOnce(Return(false));
	EXPECT_CALL(*stepper, SetDirection(true));
	EXPECT_CALL(*stepper, SetSpeed(100));
	auto command = Start(true, 100);
	state.ProcessCommand(command);
	EXPECT_EQ(state.GetState(), States::ACCELERATING);
}

TEST(State, Start_with_Zero_Speed_While_Stopped_Remains_Stopped)
{
	std::shared_ptr<TestStepper> stepper = std::make_shared<TestStepper>();
	State state(stepper);
	EXPECT_CALL(*stepper, GetCurrentSpeed()).WillOnce(Return(0));
	auto command = Start(true, 0);
	state.ProcessCommand(command);
	EXPECT_EQ(state.GetState(), States::STOPPED);
}

TEST(State, Start_While_Moving_With_Greater_Speed_Than_Current_Accelerates)
{
	std::shared_ptr<TestStepper> stepper = std::make_shared<TestStepper>();
	State state(stepper);
	EXPECT_CALL(*stepper, GetCurrentSpeed()).WillOnce(Return(0));
	EXPECT_CALL(*stepper, GetTargetSpeed()).WillOnce(Return(0));
	EXPECT_CALL(*stepper, GetDirection()).WillRepeatedly(Return(true));
	EXPECT_CALL(*stepper, GetTargetDirection()).WillRepeatedly(Return(true));
	EXPECT_CALL(*stepper, SetSpeed(50));
	auto command = Start(true, 50);
	state.ProcessCommand(command);
	EXPECT_EQ(state.GetState(), States::ACCELERATING);

	EXPECT_CALL(*stepper, GetCurrentSpeed()).WillOnce(Return(0));
	EXPECT_CALL(*stepper, GetTargetSpeed()).WillOnce(Return(50));

	state.Run();
	EXPECT_EQ(state.GetState(), States::ACCELERATING);

	// Should now be in the ACCELERATING STATE

	// Send another start command with a higher speed

	EXPECT_CALL(*stepper, SetSpeed(1000));
	// partially up to speed, so it shouldn't be coasting
	EXPECT_CALL(*stepper, GetCurrentSpeed()).WillOnce(Return(20));
	command = Start(true, 1000);
	state.ProcessCommand(command);
	EXPECT_EQ(state.GetState(), States::ACCELERATING);

	state.Run();
	EXPECT_EQ(state.GetState(), States::ACCELERATING);
}

TEST(State, Start_While_Accelerating_With_Less_Speed_Than_Current_Decelerates)
{
	std::shared_ptr<TestStepper> stepper = std::make_shared<TestStepper>();
	State state(stepper);
	EXPECT_CALL(*stepper, GetCurrentSpeed()).WillOnce(Return(0));
	EXPECT_CALL(*stepper, GetDirection()).WillOnce(Return(true));
	EXPECT_CALL(*stepper, SetSpeed(500));
	auto command = Start(true, 500);
	state.ProcessCommand(command);
	EXPECT_EQ(state.GetState(), States::ACCELERATING);
	state.Run();
	EXPECT_EQ(state.GetState(), States::ACCELERATING);

	// partially up to speed, so it shouldn't be coasting
	EXPECT_CALL(*stepper, GetCurrentSpeed()).WillOnce(Return(200));
	EXPECT_CALL(*stepper, SetSpeed(100));
	command = Start(true, 100);
	state.ProcessCommand(command);
	EXPECT_EQ(state.GetState(), States::DECELERATING);

	EXPECT_CALL(*stepper, GetCurrentSpeed()).WillOnce(Return(200));
	state.Run();
	EXPECT_EQ(state.GetState(), States::DECELERATING);
}

// TEST(State, Stop_While_Moving_Decelerates)
// {
// 	std::shared_ptr<TestStepper> stepper = std::make_shared<TestStepper>();
// 	State state(stepper);
// 	EXPECT_CALL(*stepper, GetCurrentSpeed()).WillOnce(Return(100));
// 	state.ProcessCommand(Start(true, 100));
// 	state.ProcessCommand(Stop());
// 	EXPECT_EQ(state.GetState(), States::DECELERATING);
// }

// TEST(State, Stop_While_Stopped_Does_Nothing)
// {
// 	std::shared_ptr<TestStepper> stepper = std::make_shared<TestStepper>();
// 	State state(stepper);
// 	state.ProcessCommand(Stop());
// 	EXPECT_EQ(state.GetState(), States::STOPPED);
// }

// TEST(State, Stop_While_Accelerating_Decelerates)
// {
// 	std::shared_ptr<TestStepper> stepper = std::make_shared<TestStepper>();
// 	State state(stepper);
// 	EXPECT_CALL(*stepper, GetCurrentSpeed()).WillOnce(Return(50));
// 	state.ProcessCommand(Start(true, 100));
// 	state.ProcessCommand(Stop());
// 	EXPECT_EQ(state.GetState(), States::DECELERATING);
// }

// TEST(State, Stop_While_Coasting_Decelerates)
// {
// 	std::shared_ptr<TestStepper> stepper = std::make_shared<TestStepper>();
// 	State state(stepper);
// 	EXPECT_CALL(*stepper, GetCurrentSpeed()).WillOnce(Return(100));
// 	state.ProcessCommand(Start(true, 100));
// 	state.ProcessCommand(Stop());
// 	EXPECT_EQ(state.GetState(), States::DECELERATING);
// }

// TEST(State, Stop_While_Decelerating_Sets_Target_Speed_To_Zero)
// {
// 	std::shared_ptr<TestStepper> stepper = std::make_shared<TestStepper>();
// 	State state(stepper);
// 	EXPECT_CALL(*stepper, GetCurrentSpeed()).WillOnce(Return(100));
// 	state.ProcessCommand(Start(true, 100));
// 	state.ProcessCommand(Stop());
// 	state.ProcessCommand(Stop());
// 	EXPECT_EQ(state.GetState(), States::DECELERATING);
// }

// TEST(State, Decelerate_While_Decelerating_Changes_Target_Speed)
// {
// 	std::shared_ptr<TestStepper> stepper = std::make_shared<TestStepper>();
// 	State state(stepper);
// 	EXPECT_CALL(*stepper, GetCurrentSpeed()).WillOnce(Return(100));
// 	state.ProcessCommand(Start(true, 100));
// 	state.ProcessCommand(Stop());
// 	state.ProcessCommand(Start(true, 50));
// 	EXPECT_EQ(state.GetState(), States::DECELERATING);
// }

// TEST(State, Setting_Lower_speed_while_Coasting_Decelerates_and_Changes_Target_Speed)
// {
// 	std::shared_ptr<TestStepper> stepper = std::make_shared<TestStepper>();
// 	State state(stepper);
// 	EXPECT_CALL(*stepper, GetCurrentSpeed()).WillOnce(Return(100));
// 	state.ProcessCommand(Start(true, 100));
// 	state.ProcessCommand(Start(true, 50));
// 	EXPECT_EQ(state.GetState(), States::DECELERATING);
// }

// TEST(State, Start_While_Changing_Direction_Changes_Direction)
// {
// 	std::shared_ptr<TestStepper> stepper = std::make_shared<TestStepper>();
// 	State state(stepper);
// 	EXPECT_CALL(*stepper, GetCurrentSpeed()).WillOnce(Return(100));
// 	EXPECT_CALL(*stepper, GetDirection()).WillOnce(Return(true));
// 	EXPECT_CALL(*stepper, SetDirection(false));
// 	state.ProcessCommand(Start(false, 100));
// 	EXPECT_EQ(state.GetState(), States::CHANGING_DIRECTION);
// }