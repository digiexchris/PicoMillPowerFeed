#include "../src/StepperState.hpp"
#include "TestCommon.hpp"
#include "TestStepper.hpp"
#include <gtest/gtest.h>
#include <memory>

using ::testing::Return;
using namespace PicoMill;

class StateTest : public ::testing::Test
{
protected:
	void SetUp() override
	{
		stepper = std::make_shared<Drivers::TestStepper>();
		time = std::make_shared<TestTime>();
		state = std::make_unique<StepperState>(stepper, time);
	}

	void TearDown() override
	{
		stepper.reset();
		time.reset();
		state.reset();
	}

	std::shared_ptr<Drivers::TestStepper> stepper;
	std::shared_ptr<TestTime> time;
	std::unique_ptr<StepperState> state;
};

TEST_F(StateTest, Start)
{
	EXPECT_CALL(*stepper, GetCurrentSpeed()).WillOnce(Return(0));
	EXPECT_CALL(*stepper, GetTargetSpeed()).WillOnce(Return(0));
	EXPECT_CALL(*stepper, GetDirection()).WillOnce(Return(false));
	EXPECT_CALL(*stepper, GetTargetDirection()).WillOnce(Return(false));
	EXPECT_CALL(*stepper, SetDirection(true));
	EXPECT_CALL(*stepper, SetSpeed(100));
	std::shared_ptr<Command> command = std::make_shared<Start>(true, 100);
	state->ProcessCommand(command);
	EXPECT_EQ(state->GetState(), States::ACCELERATING);
}

TEST_F(StateTest, Start_With_Same_Direction_Does_Not_Change_Direction)
{
	EXPECT_CALL(*stepper, GetCurrentSpeed()).WillOnce(Return(0));
	EXPECT_CALL(*stepper, GetTargetSpeed()).WillOnce(Return(0));
	EXPECT_CALL(*stepper, GetDirection()).WillOnce(Return(false));
	EXPECT_CALL(*stepper, GetTargetDirection()).WillOnce(Return(false));
	EXPECT_CALL(*stepper, SetDirection(true)).Times(0);
	EXPECT_CALL(*stepper, SetSpeed(100));
	std::shared_ptr<Command> command = std::make_shared<Start>(false, 100);
	state->ProcessCommand(command);
	EXPECT_EQ(state->GetState(), States::ACCELERATING);
}

TEST_F(StateTest, Start_with_Zero_Speed_While_Stopped_Remains_Stopped)
{
	EXPECT_CALL(*stepper, GetCurrentSpeed()).WillOnce(Return(0));
	EXPECT_CALL(*stepper, GetTargetSpeed()).WillOnce(Return(0));
	EXPECT_CALL(*stepper, GetDirection()).WillOnce(Return(false));
	EXPECT_CALL(*stepper, GetTargetDirection()).WillOnce(Return(false));
	EXPECT_CALL(*stepper, SetDirection(true));
	std::shared_ptr<Command> command = std::make_shared<Start>(true, 0);
	state->ProcessCommand(command);
	EXPECT_EQ(state->GetState(), States::STOPPED);
}

TEST_F(StateTest, Start_While_Accelerating_With_Greater_Speed_Than_Current_Accelerates_And_Coasts)
{
	EXPECT_CALL(*stepper, GetCurrentSpeed()).WillOnce(Return(0));
	EXPECT_CALL(*stepper, GetTargetSpeed()).WillOnce(Return(0));
	EXPECT_CALL(*stepper, GetDirection()).WillRepeatedly(Return(true));
	EXPECT_CALL(*stepper, GetTargetDirection()).WillRepeatedly(Return(true));
	EXPECT_CALL(*stepper, SetSpeed(50));
	std::shared_ptr<Command> command = std::make_shared<Start>(true, 50);
	state->ProcessCommand(command);
	EXPECT_EQ(state->GetState(), States::ACCELERATING);

	EXPECT_CALL(*stepper, GetCurrentSpeed()).WillOnce(Return(0));
	EXPECT_CALL(*stepper, GetTargetSpeed()).WillOnce(Return(50));

	state->Run();
	EXPECT_EQ(state->GetState(), States::ACCELERATING);

	// Should now be in the ACCELERATING STATE

	// Send another start command with a higher speed

	EXPECT_CALL(*stepper, SetSpeed(1000));
	// partially up to speed, so it shouldn't be coasting
	EXPECT_CALL(*stepper, GetCurrentSpeed()).WillOnce(Return(20));
	EXPECT_CALL(*stepper, GetTargetSpeed()).WillOnce(Return(50));
	command = std::make_shared<Start>(true, 1000);
	state->ProcessCommand(command);
	EXPECT_EQ(state->GetState(), States::ACCELERATING);

	EXPECT_CALL(*stepper, GetCurrentSpeed()).WillOnce(Return(30));
	EXPECT_CALL(*stepper, GetTargetSpeed()).WillOnce(Return(1000));

	state->Run();
	EXPECT_EQ(state->GetState(), States::ACCELERATING);

	EXPECT_CALL(*stepper, GetCurrentSpeed()).WillOnce(Return(1000));
	EXPECT_CALL(*stepper, GetTargetSpeed()).WillOnce(Return(1000));

	state->Run();
	EXPECT_EQ(state->GetState(), States::COASTING);
}

TEST_F(StateTest, Start_While_Stopping_Accelerates_And_Coasts)
{
	EXPECT_CALL(*stepper, GetCurrentSpeed()).WillOnce(Return(0));
	EXPECT_CALL(*stepper, GetTargetSpeed()).WillOnce(Return(0));
	EXPECT_CALL(*stepper, GetDirection()).WillRepeatedly(Return(true));
	EXPECT_CALL(*stepper, GetTargetDirection()).WillRepeatedly(Return(true));
	EXPECT_CALL(*stepper, SetSpeed(50));
	std::shared_ptr<Command> command = std::make_shared<Start>(true, 50);
	state->ProcessCommand(command);
	EXPECT_EQ(state->GetState(), States::ACCELERATING);

	EXPECT_CALL(*stepper, GetCurrentSpeed()).WillOnce(Return(0));
	EXPECT_CALL(*stepper, GetTargetSpeed()).WillOnce(Return(50));
	state->Run();
	EXPECT_EQ(state->GetState(), States::ACCELERATING);

	// Should now be in the ACCELERATING STATE

	EXPECT_CALL(*stepper, SetSpeed(0));
	command = std::make_shared<Stop>();
	state->ProcessCommand(command);
	EXPECT_EQ(state->GetState(), States::DECELERATING);

	EXPECT_CALL(*stepper, GetCurrentSpeed()).WillOnce(Return(10));
	EXPECT_CALL(*stepper, GetTargetSpeed()).WillOnce(Return(0));
	state->Run();
	EXPECT_EQ(state->GetState(), States::DECELERATING);

	EXPECT_CALL(*stepper, GetCurrentSpeed()).WillOnce(Return(5));
	EXPECT_CALL(*stepper, GetTargetSpeed()).WillOnce(Return(0));
	EXPECT_CALL(*stepper, SetSpeed(150));
	command = std::make_shared<Start>(true, 150);
	state->ProcessCommand(command);
	EXPECT_EQ(state->GetState(), States::ACCELERATING);

	EXPECT_CALL(*stepper, GetCurrentSpeed()).WillOnce(Return(150));
	EXPECT_CALL(*stepper, GetTargetSpeed()).WillOnce(Return(150));
	state->Run();
	EXPECT_EQ(state->GetState(), States::COASTING);
}

TEST_F(StateTest, Start_While_Accelerating_With_Less_Speed_Than_Current_Decelerates_And_Coasts)
{
	EXPECT_CALL(*stepper, GetCurrentSpeed()).WillOnce(Return(0));
	EXPECT_CALL(*stepper, GetTargetSpeed()).WillOnce(Return(0));
	EXPECT_CALL(*stepper, GetDirection()).WillRepeatedly(Return(true));
	EXPECT_CALL(*stepper, GetTargetDirection()).WillRepeatedly(Return(true));
	EXPECT_CALL(*stepper, SetSpeed(500));
	std::shared_ptr<Command> command = std::make_shared<Start>(true, 500);
	state->ProcessCommand(command);
	EXPECT_EQ(state->GetState(), States::ACCELERATING);

	EXPECT_CALL(*stepper, GetCurrentSpeed()).WillOnce(Return(0));
	EXPECT_CALL(*stepper, GetTargetSpeed()).WillOnce(Return(400));

	state->Run();
	EXPECT_EQ(state->GetState(), States::ACCELERATING);

	EXPECT_CALL(*stepper, SetSpeed(100));
	// partially up to speed, so it shouldn't be coasting
	EXPECT_CALL(*stepper, GetCurrentSpeed()).WillOnce(Return(450));
	EXPECT_CALL(*stepper, GetTargetSpeed()).WillOnce(Return(100));
	command = std::make_shared<Start>(true, 100);
	state->ProcessCommand(command);
	EXPECT_EQ(state->GetState(), States::DECELERATING);

	EXPECT_CALL(*stepper, GetCurrentSpeed()).WillOnce(Return(200));
	EXPECT_CALL(*stepper, GetTargetSpeed()).WillOnce(Return(100));

	state->Run();
	EXPECT_EQ(state->GetState(), States::DECELERATING);

	EXPECT_CALL(*stepper, GetCurrentSpeed()).WillOnce(Return(100));
	EXPECT_CALL(*stepper, GetTargetSpeed()).WillOnce(Return(100));

	state->Run();
	EXPECT_EQ(state->GetState(), States::COASTING);
}

TEST_F(StateTest, Stop_While_Accelerating_Decelerates_And_Stops)
{
	EXPECT_CALL(*stepper, GetCurrentSpeed()).WillOnce(Return(0));
	EXPECT_CALL(*stepper, GetTargetSpeed()).WillOnce(Return(0));
	EXPECT_CALL(*stepper, GetDirection()).WillRepeatedly(Return(true));
	EXPECT_CALL(*stepper, GetTargetDirection()).WillRepeatedly(Return(true));
	EXPECT_CALL(*stepper, SetSpeed(500));
	std::shared_ptr<Command> command = std::make_shared<Start>(true, 500);
	state->ProcessCommand(command);
	EXPECT_EQ(state->GetState(), States::ACCELERATING);

	EXPECT_CALL(*stepper, GetCurrentSpeed()).WillOnce(Return(0));
	EXPECT_CALL(*stepper, GetTargetSpeed()).WillOnce(Return(500));

	state->Run();
	EXPECT_EQ(state->GetState(), States::ACCELERATING);

	EXPECT_CALL(*stepper, SetSpeed(0));
	command.reset();
	command = std::make_shared<Stop>();
	state->ProcessCommand(command);
	EXPECT_EQ(state->GetState(), States::DECELERATING);

	EXPECT_CALL(*stepper, GetCurrentSpeed()).WillOnce(Return(200));
	EXPECT_CALL(*stepper, GetTargetSpeed()).WillOnce(Return(0));

	state->Run();
	EXPECT_EQ(state->GetState(), States::DECELERATING);

	EXPECT_CALL(*stepper, GetCurrentSpeed()).WillOnce(Return(0));
	EXPECT_CALL(*stepper, GetTargetSpeed()).WillOnce(Return(0));

	state->Run();
	EXPECT_EQ(state->GetState(), States::STOPPED);
}

TEST_F(StateTest, Stop_While_Coasting_Decelerates_And_Stops)
{
	EXPECT_CALL(*stepper, GetCurrentSpeed()).WillOnce(Return(0));
	EXPECT_CALL(*stepper, GetTargetSpeed()).WillOnce(Return(0));
	EXPECT_CALL(*stepper, GetDirection()).WillRepeatedly(Return(true));
	EXPECT_CALL(*stepper, GetTargetDirection()).WillRepeatedly(Return(true));
	EXPECT_CALL(*stepper, SetSpeed(500));
	std::shared_ptr<Command> command = std::make_shared<Start>(true, 500);
	state->ProcessCommand(command);
	EXPECT_EQ(state->GetState(), States::ACCELERATING);

	EXPECT_CALL(*stepper, GetCurrentSpeed()).WillOnce(Return(500));
	EXPECT_CALL(*stepper, GetTargetSpeed()).WillOnce(Return(500));

	state->Run();
	EXPECT_EQ(state->GetState(), States::COASTING);

	EXPECT_CALL(*stepper, SetSpeed(0));
	// partially up to speed, so it shouldn't be coasting
	command = std::make_shared<Stop>();
	state->ProcessCommand(command);
	EXPECT_EQ(state->GetState(), States::DECELERATING);

	EXPECT_CALL(*stepper, GetCurrentSpeed()).WillOnce(Return(200));
	EXPECT_CALL(*stepper, GetTargetSpeed()).WillOnce(Return(0));

	state->Run();
	EXPECT_EQ(state->GetState(), States::DECELERATING);

	EXPECT_CALL(*stepper, GetCurrentSpeed()).WillOnce(Return(0));
	EXPECT_CALL(*stepper, GetTargetSpeed()).WillOnce(Return(0));

	state->Run();
	EXPECT_EQ(state->GetState(), States::STOPPED);
}

TEST_F(StateTest, Stop_While_Decelerating_Decelerates_And_Stops)
{
	EXPECT_CALL(*stepper, GetCurrentSpeed()).WillOnce(Return(0));
	EXPECT_CALL(*stepper, GetTargetSpeed()).WillOnce(Return(0));
	EXPECT_CALL(*stepper, GetDirection()).WillRepeatedly(Return(true));
	EXPECT_CALL(*stepper, GetTargetDirection()).WillRepeatedly(Return(true));
	EXPECT_CALL(*stepper, SetSpeed(500));
	std::shared_ptr<Command> command = std::make_shared<Start>(true, 500);
	state->ProcessCommand(command);
	EXPECT_EQ(state->GetState(), States::ACCELERATING);

	EXPECT_CALL(*stepper, GetCurrentSpeed()).WillOnce(Return(500));
	EXPECT_CALL(*stepper, GetTargetSpeed()).WillOnce(Return(500));

	state->Run();
	EXPECT_EQ(state->GetState(), States::COASTING);

	EXPECT_CALL(*stepper, SetSpeed(100));
	command = std::make_shared<ChangeSpeed>(100);
	EXPECT_CALL(*stepper, GetCurrentSpeed()).WillOnce(Return(500));
	EXPECT_CALL(*stepper, GetTargetSpeed()).WillOnce(Return(500));
	state->ProcessCommand(command);
	EXPECT_EQ(state->GetState(), States::DECELERATING);

	EXPECT_CALL(*stepper, SetSpeed(0));
	// partially up to speed, so it shouldn't be coasting
	command = std::make_shared<Stop>();
	state->ProcessCommand(command);
	EXPECT_EQ(state->GetState(), States::DECELERATING);

	EXPECT_CALL(*stepper, GetCurrentSpeed()).WillOnce(Return(200));
	EXPECT_CALL(*stepper, GetTargetSpeed()).WillOnce(Return(0));

	state->Run();
	EXPECT_EQ(state->GetState(), States::DECELERATING);

	EXPECT_CALL(*stepper, GetCurrentSpeed()).WillOnce(Return(0));
	EXPECT_CALL(*stepper, GetTargetSpeed()).WillOnce(Return(0));

	state->Run();
	EXPECT_EQ(state->GetState(), States::STOPPED);
}

TEST_F(StateTest, Stop_While_Stopped_Remains_Stopped_And_Disable_After_Timeout)
{
	EXPECT_CALL(*stepper, SetSpeed(0)).Times(1);
	std::shared_ptr<Command> command = std::make_shared<Stop>();
	state->ProcessCommand(command);
	EXPECT_EQ(state->GetState(), States::STOPPED);

	EXPECT_CALL(*stepper, GetCurrentSpeed()).WillOnce(Return(0));
	EXPECT_CALL(*stepper, GetTargetSpeed()).WillOnce(Return(0));
	EXPECT_CALL(*stepper, GetDirection()).WillRepeatedly(Return(true));
	EXPECT_CALL(*stepper, GetTargetDirection()).WillRepeatedly(Return(true));
	EXPECT_CALL(*stepper, IsEnabled()).WillOnce(Return(true));
	EXPECT_CALL(*time, GetCurrentTimeInMilliseconds()).WillOnce(Return(3));
	state->Run();
	EXPECT_EQ(state->GetState(), States::STOPPED);

	EXPECT_CALL(*stepper, GetCurrentSpeed()).WillOnce(Return(0));
	EXPECT_CALL(*stepper, GetTargetSpeed()).WillOnce(Return(0));
	EXPECT_CALL(*stepper, GetDirection()).WillRepeatedly(Return(true));
	EXPECT_CALL(*stepper, GetTargetDirection()).WillRepeatedly(Return(true));
	EXPECT_CALL(*time, GetCurrentTimeInMilliseconds()).WillOnce(Return(1010));
	EXPECT_CALL(*stepper, IsEnabled()).WillOnce(Return(true));
	EXPECT_CALL(*stepper, Disable()).Times(1);
	state->Run();
	EXPECT_EQ(state->GetState(), States::STOPPED);
}

TEST_F(StateTest, Decelerate_While_Decelerating_Changes_Target_Speed_And_Coasts)
{
	EXPECT_CALL(*stepper, GetCurrentSpeed()).WillOnce(Return(0));
	EXPECT_CALL(*stepper, GetTargetSpeed()).WillOnce(Return(0));
	EXPECT_CALL(*stepper, GetDirection()).WillRepeatedly(Return(true));
	EXPECT_CALL(*stepper, GetTargetDirection()).WillRepeatedly(Return(true));
	EXPECT_CALL(*stepper, SetSpeed(100));
	std::shared_ptr<Command> command = std::make_shared<Start>(true, 100);
	state->ProcessCommand(command);
	EXPECT_EQ(state->GetState(), States::ACCELERATING);

	EXPECT_CALL(*stepper, GetCurrentSpeed()).WillOnce(Return(1000));
	EXPECT_CALL(*stepper, GetTargetSpeed()).WillOnce(Return(1000));
	state->Run();
	EXPECT_EQ(state->GetState(), States::COASTING);

	EXPECT_CALL(*stepper, SetSpeed(500));
	command = std::make_shared<ChangeSpeed>(500);
	EXPECT_CALL(*stepper, GetCurrentSpeed()).WillOnce(Return(1000));
	EXPECT_CALL(*stepper, GetTargetSpeed()).WillOnce(Return(500));
	state->ProcessCommand(command);
	EXPECT_EQ(state->GetState(), States::DECELERATING);

	EXPECT_CALL(*stepper, SetSpeed(200));
	command = std::make_shared<ChangeSpeed>(200);
	EXPECT_CALL(*stepper, GetCurrentSpeed()).WillOnce(Return(600));
	EXPECT_CALL(*stepper, GetTargetSpeed()).WillOnce(Return(500));
	state->ProcessCommand(command);
	EXPECT_EQ(state->GetState(), States::DECELERATING);

	EXPECT_CALL(*stepper, GetCurrentSpeed()).WillOnce(Return(400));
	EXPECT_CALL(*stepper, GetTargetSpeed()).WillOnce(Return(200));
	state->Run();
	EXPECT_EQ(state->GetState(), States::DECELERATING);

	EXPECT_CALL(*stepper, SetSpeed(100));
	EXPECT_CALL(*stepper, GetCurrentSpeed()).WillOnce(Return(300));
	EXPECT_CALL(*stepper, GetTargetSpeed()).WillOnce(Return(200));
	// partially up to speed, so it shouldn't be coasting
	command = std::make_shared<ChangeSpeed>(100);
	state->ProcessCommand(command);
	EXPECT_EQ(state->GetState(), States::DECELERATING);

	EXPECT_CALL(*stepper, GetCurrentSpeed()).WillOnce(Return(250));
	EXPECT_CALL(*stepper, GetTargetSpeed()).WillOnce(Return(100));
	state->Run();
	EXPECT_EQ(state->GetState(), States::DECELERATING);

	EXPECT_CALL(*stepper, GetCurrentSpeed()).WillOnce(Return(100));
	EXPECT_CALL(*stepper, GetTargetSpeed()).WillOnce(Return(100));
	state->Run();
	EXPECT_EQ(state->GetState(), States::COASTING);
}

TEST_F(StateTest, Start_While_Accelerating_with_a_different_direction_changes_direction_accelerates_and_coasts)
{
	EXPECT_CALL(*stepper, GetCurrentSpeed()).WillOnce(Return(0));
	EXPECT_CALL(*stepper, GetTargetSpeed()).WillOnce(Return(0));
	EXPECT_CALL(*stepper, GetDirection()).WillRepeatedly(Return(true));
	EXPECT_CALL(*stepper, GetTargetDirection()).WillRepeatedly(Return(true));
	EXPECT_CALL(*stepper, SetSpeed(50));
	std::shared_ptr<Command> command = std::make_shared<Start>(true, 50);
	state->ProcessCommand(command);
	EXPECT_EQ(state->GetState(), States::ACCELERATING);

	EXPECT_CALL(*stepper, GetCurrentSpeed()).WillOnce(Return(0));
	EXPECT_CALL(*stepper, GetTargetSpeed()).WillOnce(Return(50));

	state->Run();
	EXPECT_EQ(state->GetState(), States::ACCELERATING);

	// Should now be in the ACCELERATING STATE

	// Send another start command with a higher speed

	EXPECT_CALL(*stepper, SetSpeed(1000));
	// partially up to speed, so it shouldn't be coasting
	EXPECT_CALL(*stepper, GetCurrentSpeed()).WillOnce(Return(20));
	EXPECT_CALL(*stepper, GetTargetSpeed()).WillOnce(Return(50));
	EXPECT_CALL(*stepper, SetDirection(false)).Times(1);
	command = std::make_shared<Start>(false, 1000);
	state->ProcessCommand(command);
	EXPECT_EQ(state->GetState(), States::CHANGING_DIRECTION);

	EXPECT_CALL(*stepper, GetCurrentSpeed()).WillOnce(Return(30));
	EXPECT_CALL(*stepper, GetTargetSpeed()).WillOnce(Return(1000));
	EXPECT_CALL(*stepper, GetDirection()).WillRepeatedly(Return(true));
	EXPECT_CALL(*stepper, GetTargetDirection()).WillRepeatedly(Return(false));
	state->Run();
	EXPECT_EQ(state->GetState(), States::CHANGING_DIRECTION);

	EXPECT_CALL(*stepper, GetCurrentSpeed()).WillOnce(Return(0));
	EXPECT_CALL(*stepper, GetTargetSpeed()).WillOnce(Return(1000));
	EXPECT_CALL(*stepper, GetDirection()).WillRepeatedly(Return(true));
	EXPECT_CALL(*stepper, GetTargetDirection()).WillRepeatedly(Return(false));
	state->Run();
	EXPECT_EQ(state->GetState(), States::CHANGING_DIRECTION);

	EXPECT_CALL(*stepper, GetCurrentSpeed()).WillOnce(Return(1));
	EXPECT_CALL(*stepper, GetTargetSpeed()).WillOnce(Return(1000));
	EXPECT_CALL(*stepper, GetDirection()).WillRepeatedly(Return(false));
	EXPECT_CALL(*stepper, GetTargetDirection()).WillRepeatedly(Return(false));
	state->Run();
	EXPECT_EQ(state->GetState(), States::ACCELERATING);

	EXPECT_CALL(*stepper, GetCurrentSpeed()).WillOnce(Return(1000));
	EXPECT_CALL(*stepper, GetTargetSpeed()).WillOnce(Return(1000));

	state->Run();
	EXPECT_EQ(state->GetState(), States::COASTING);
}

TEST_F(StateTest, Start_With_Different_Direction_While_Stopping_Changes_Direciton_Accelerates_And_Coasts)
{
	EXPECT_CALL(*stepper, GetCurrentSpeed()).WillOnce(Return(0));
	EXPECT_CALL(*stepper, GetTargetSpeed()).WillOnce(Return(0));
	EXPECT_CALL(*stepper, GetDirection()).WillRepeatedly(Return(true));
	EXPECT_CALL(*stepper, GetTargetDirection()).WillRepeatedly(Return(true));
	EXPECT_CALL(*stepper, SetSpeed(50));
	std::shared_ptr<Command> command = std::make_shared<Start>(true, 50);
	state->ProcessCommand(command);
	EXPECT_EQ(state->GetState(), States::ACCELERATING);

	EXPECT_CALL(*stepper, GetCurrentSpeed()).WillOnce(Return(0));
	EXPECT_CALL(*stepper, GetTargetSpeed()).WillOnce(Return(50));
	state->Run();
	EXPECT_EQ(state->GetState(), States::ACCELERATING);

	// Should now be in the ACCELERATING STATE

	EXPECT_CALL(*stepper, SetSpeed(0));
	command = std::make_shared<Stop>();
	state->ProcessCommand(command);
	EXPECT_EQ(state->GetState(), States::DECELERATING);

	EXPECT_CALL(*stepper, GetCurrentSpeed()).WillOnce(Return(10));
	EXPECT_CALL(*stepper, GetTargetSpeed()).WillOnce(Return(0));
	state->Run();
	EXPECT_EQ(state->GetState(), States::DECELERATING);

	EXPECT_CALL(*stepper, GetCurrentSpeed()).WillOnce(Return(5));
	EXPECT_CALL(*stepper, GetTargetSpeed()).WillOnce(Return(0));
	EXPECT_CALL(*stepper, SetSpeed(1000));
	EXPECT_CALL(*stepper, SetDirection(false));
	command = std::make_shared<Start>(false, 1000);
	state->ProcessCommand(command);
	EXPECT_EQ(state->GetState(), States::CHANGING_DIRECTION);

	EXPECT_CALL(*stepper, GetCurrentSpeed()).WillOnce(Return(2));
	EXPECT_CALL(*stepper, GetTargetSpeed()).WillOnce(Return(1000));
	EXPECT_CALL(*stepper, GetDirection()).WillRepeatedly(Return(true));
	EXPECT_CALL(*stepper, GetTargetDirection()).WillRepeatedly(Return(false));
	state->Run();
	EXPECT_EQ(state->GetState(), States::CHANGING_DIRECTION);

	EXPECT_CALL(*stepper, GetCurrentSpeed()).WillOnce(Return(0));
	EXPECT_CALL(*stepper, GetTargetSpeed()).WillOnce(Return(1000));
	EXPECT_CALL(*stepper, GetDirection()).WillRepeatedly(Return(true));
	EXPECT_CALL(*stepper, GetTargetDirection()).WillRepeatedly(Return(false));
	state->Run();
	EXPECT_EQ(state->GetState(), States::CHANGING_DIRECTION);

	EXPECT_CALL(*stepper, GetCurrentSpeed()).WillOnce(Return(1));
	EXPECT_CALL(*stepper, GetTargetSpeed()).WillOnce(Return(1000));
	EXPECT_CALL(*stepper, GetDirection()).WillRepeatedly(Return(false));
	EXPECT_CALL(*stepper, GetTargetDirection()).WillRepeatedly(Return(false));
	state->Run();
	EXPECT_EQ(state->GetState(), States::ACCELERATING);

	EXPECT_CALL(*stepper, GetCurrentSpeed()).WillOnce(Return(1000));
	EXPECT_CALL(*stepper, GetTargetSpeed()).WillOnce(Return(1000));

	state->Run();
	EXPECT_EQ(state->GetState(), States::COASTING);
}