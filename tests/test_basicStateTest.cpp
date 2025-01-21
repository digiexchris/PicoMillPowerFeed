#include "../src/State.hpp"
#include "TestStepper.hpp"
#include <gtest/gtest.h>

TEST(State, Start)
{
	std::shared_ptr<IStepper> stepper = std::make_shared<TestStepper>();
	State state(stepper);
	state.ProcessCommand(Start(true, 100));
	EXPECT_EQ(state.GetState(), States::ACCELERATING);
}

TEST(State, Start_with_Zero_Speed_While_Stopped_Remains_Stopped)
{
	std::shared_ptr<IStepper> stepper = std::make_shared<TestStepper>();
	State state(stepper);
	state.ProcessCommand(Start(true, 0));
	
	EXPECT_EQ(state.GetState(), States::STOPPED);
}

TEST(State, Start_While_Moving_With_Greater_Speed_Than_Current_Accelerates)
{
	std::shared_ptr<IStepper> stepper = std::make_shared<TestStepper>();
	State state(stepper);
	state.ProcessCommand(Start(true, 100));
	// state.ProcessCommand(Start(true, 100));
	// state.ProcessCommand(Start(true, 200));
	EXPECT_EQ(state.GetState(), States::ACCELERATING);
}

TEST(State, Start_While_Moving_With_Less_Speed_Than_Current_Decelerates)
{
	std::shared_ptr<IStepper> stepper = std::make_shared<TestStepper>();
	State state(stepper);
	// state.ProcessCommand(Start(true, 100));
	// state.ProcessCommand(Start(true, 50));
	EXPECT_EQ(state.GetState(), States::DECELERATING);
}

TEST(State, Stop_While_Moving_Decelerates)
{
	std::shared_ptr<IStepper> stepper = std::make_shared<TestStepper>();
	State state(stepper);
	// state.ProcessCommand(Start(true, 100));
	// state.ProcessCommand(Stop());
	EXPECT_EQ(state.GetState(), States::DECELERATING);
}

TEST(State, Stop_While_Stopped_Does_Nothing)
{
	std::shared_ptr<IStepper> stepper = std::make_shared<TestStepper>();
	State state(stepper);
	// state.ProcessCommand(Stop());
	EXPECT_EQ(state.GetState(), States::STOPPED);
}

TEST(State, Stop_While_Accelerating_Decelerates)
{
	std::shared_ptr<IStepper> stepper = std::make_shared<TestStepper>();
	State state(stepper);
	// state.ProcessCommand(Start(true, 100));
	// state.ProcessCommand(Stop());
	EXPECT_EQ(state.GetState(), States::DECELERATING);
}

TEST(State, Stop_While_Coasting_Decelerates)
{
	std::shared_ptr<IStepper> stepper = std::make_shared<TestStepper>();
	State state(stepper);
	// state.ProcessCommand(Start(true, 100));
	// state.ProcessCommand(Stop());
	EXPECT_EQ(state.GetState(), States::DECELERATING);
}

TEST(State, Stop_While_Decelerating_Sets_Target_Speed_To_Zero)
{
	std::shared_ptr<IStepper> stepper = std::make_shared<TestStepper>();
	State state(stepper);
	// state.ProcessCommand(Start(true, 100));
	// state.ProcessCommand(Stop());
	// state.ProcessCommand(Stop());
	EXPECT_EQ(state.GetState(), States::DECELERATING);
}

TEST(State, Decelerate_While_Decelerating_Changes_Target_Speed)
{
	std::shared_ptr<IStepper> stepper = std::make_shared<TestStepper>();
	State state(stepper);
	// state.ProcessCommand(Start(true, 100));
	// state.ProcessCommand(Stop());
	// state.ProcessCommand(Stop());
	// state.ProcessCommand(Start(true, 50));
	EXPECT_EQ(state.GetState(), States::DECELERATING);
}

TEST(State, Setting_Lower_speed_while_Coasting_Decelerates_and_Changes_Target_Speed)
{
	std::shared_ptr<IStepper> stepper = std::make_shared<TestStepper>();
	State state(stepper);
	// state.ProcessCommand(Start(true, 100));
	// state.ProcessCommand(Stop());
	// state.ProcessCommand(Start(true, 50));
	EXPECT_EQ(state.GetState(), States::DECELERATING);
}