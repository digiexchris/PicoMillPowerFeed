#include "../src/Settings.hxx"
#include "TestDisplay.hpp"
#include "textRenderer/12x16_font.h"
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
			myDisplay = std::make_shared<MockDisplayHal>(mySettings);
		}

		void SetUp() override
		{
			myDisplay = std::make_shared<MockDisplayHal>(mySettings);
		}

		void TearDown() override
		{
			myDisplay.reset();
		}

		std::shared_ptr<MockDisplayHal> myDisplay;
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
		EXPECT_CALL(*myDisplay, DrawText(testing::StrEq("23.1 ipm"), _, _, _)).Times(1);
		myDisplay->ToggleUnits();
		myDisplay->DrawSpeed(speed);
	}

} // namespace