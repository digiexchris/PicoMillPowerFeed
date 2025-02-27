#include "../src/Settings.hxx"
#include "TestDisplay.hpp"
#include "gmock/gmock.h"
#include <gtest/gtest.h>

using ::testing::_;
using ::testing::Eq;

namespace PowerFeed
{
	class DisplayTest : public ::testing::Test
	{
	protected:
		DisplayTest()
		{
			mySettings = std::make_shared<SettingsManager>();
		}

		void SetUp() override
		{
			myDisplay = std::unique_ptr<TestDisplay>(new TestDisplay(mySettings.get(), font_5x8));
		}

		void TearDown() override
		{
			myDisplay.reset();
		}

		std::unique_ptr<TestDisplay> myDisplay;
		std::shared_ptr<SettingsManager> mySettings;
	};

	TEST_F(DisplayTest, DrawSpeedDefaultUnits)
	{
		uint32_t speed = 10000;
		EXPECT_CALL(*myDisplay, DrawText(testing::StrEq("587.2 mm "), _, _, _)).Times(1);
		myDisplay->DrawSpeed(speed);
	}

	TEST_F(DisplayTest, DrawSpeedInchUnits)
	{
		uint32_t speed = 10000;
		// Toggle units first
		myDisplay->ToggleUnits();

		// Then expect DrawText to be called with inch units
		EXPECT_CALL(*myDisplay, DrawText(testing::StrEq("23.1 ipm"), font_5x8, _, _)).Times(1);

		// Call the method we want to test
		myDisplay->DrawSpeed(speed);
	}

} // namespace

TEST(BasicTest, SimpleAssertion)
{
	EXPECT_EQ(2, 1 + 1);
}