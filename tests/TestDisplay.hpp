#pragma once

#include "../src/Display.hpp"
#include <gmock/gmock.h>

namespace PicoMill
{
	class TestDisplay : public Display
	{
	public:
		MOCK_METHOD(void, Clear, (), (override));
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
		MOCK_METHOD(void, Clear, (), (override));
		MOCK_METHOD(void, DrawText, (const char *text, const unsigned char *font, uint16_t x, uint16_t y), (override));
		MOCK_METHOD(void, DrawImage, (const unsigned char *image, uint16_t x, uint16_t y, uint16_t width, uint16_t height), (override));
	};
} // namespace PicoMill