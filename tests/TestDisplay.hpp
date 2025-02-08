#pragma once

#include "../src/Display.hxx"
#include <gmock/gmock.h>

namespace PowerFeed
{
	class TestDisplay : public Display
	{
	public:
		MOCK_METHOD(void, Clear, (), (override));
		MOCK_METHOD(void, DrawText, (const char *text, const unsigned char *font, uint16_t x, uint16_t y), (override));
		MOCK_METHOD(void, DrawImage, (const unsigned char *image, uint16_t x, uint16_t y, uint16_t width, uint16_t height), (override));
	};

} // namespace PowerFeed