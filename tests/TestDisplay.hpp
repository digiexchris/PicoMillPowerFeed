#pragma once

#include "../src/Display.hxx"
#include <gmock/gmock.h>

namespace PowerFeed
{
	class TestDisplay : public Display
	{
	public:
		TestDisplay(std::shared_ptr<SettingsManager> settings) : Display(settings) {}
		MOCK_METHOD(void, ClearBuffer, (), (override));
		MOCK_METHOD(void, Refresh, (), (override));
		MOCK_METHOD(void, WriteBuffer, (), (override));
		MOCK_METHOD(void, DrawText, (const char *text, const unsigned char *font, uint16_t x, uint16_t y), (override));
		MOCK_METHOD(void, DrawImage, (const unsigned char *image, uint16_t x, uint16_t y, uint16_t width, uint16_t height), (override));

		MOCK_METHOD(void, DrawStart, (), (override));
		MOCK_METHOD(void, DrawMovingLeft, (), (override));
		MOCK_METHOD(void, DrawMovingRight, (), (override));
		MOCK_METHOD(void, DrawStopping, (), (override));
		MOCK_METHOD(void, DrawStopped, (), (override));
		MOCK_METHOD(void, DrawRapidLeft, (), (override));
		MOCK_METHOD(void, DrawRapidRight, (), (override));
		MOCK_METHOD(void, ToggleUnits, (), (override));
		MOCK_METHOD(void, DrawCenteredText, (const char *text, const unsigned char *font, uint16_t y), (override));

		MOCK_METHOD(void, DrawSpeed, (uint32_t speed), (override));
	};

	class MockDisplayHal : public Display
	{
	public:
		MockDisplayHal(std::shared_ptr<SettingsManager> settings) : Display(settings)
		{
		}
		MOCK_METHOD(void, ClearBuffer, (), (override));
		MOCK_METHOD(void, Refresh, (), (override));
		MOCK_METHOD(void, WriteBuffer, (), (override));
		MOCK_METHOD(void, DrawText, (const char *text, const unsigned char *font, uint16_t x, uint16_t y), (override));
		MOCK_METHOD(void, DrawImage, (const unsigned char *image, uint16_t x, uint16_t y, uint16_t width, uint16_t height), (override));
	};
} // namespace