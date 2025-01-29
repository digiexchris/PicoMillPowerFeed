#include "Display.hpp"
#include "Helpers.hpp"
#include "icons.hpp"
#include "textRenderer/12x16_font.h"
#include <cstring>
#include <task.h>

namespace PicoMill
{

	const char *IPM = "IPM";
	const char *MMPM = "MM ";

	void Display::DrawStart()
	{
		Clear();

		const char *top = "POWER";
		DrawCenteredText(top, font_12x16, 16);

		const char *bottom = "FEED";
		DrawCenteredText(bottom, font_12x16, 32);

		vTaskDelay(MS_TO_TICKS(250));
		Clear();

		DrawSpeed(0);
	}

	void Display::ToggleUnits()
	{
		myUnits = myUnits == Units::Millimeter ? Units::Inch : Units::Millimeter;
	}

	void Display::DrawMovingLeft()
	{
		DrawImage(moveleft32, leftX, 0, moveleft32WidthPixels, moveleft32HeightPixels);
	}

	void Display::DrawMovingRight()
	{
		DrawImage(moveright32, rightX, 0, moveright32WidthPixels, moveright32HeightPixels);
	}

	void Display::DrawStopping()
	{
		DrawImage(stop32inverted, leftX, 0, stop32invertedWidthPixels, stop32invertedHeightPixels);
		DrawImage(stop32inverted, rightX, 0, stop32invertedWidthPixels, stop32invertedHeightPixels);
	}

	void Display::DrawStopped()
	{
		DrawImage(stop32, leftX, 0, stop32WidthPixels, stop32HeightPixels);
		DrawImage(stop32, rightX, 0, stop32WidthPixels, stop32HeightPixels);
	}

	void Display::DrawRapidLeft()
	{
		DrawImage(rapidleft32, leftX, 0, rapidleft32WidthPixels, rapidleft32HeightPixels);
	}

	void Display::DrawRapidRight()
	{
		DrawImage(rapidright32, rightX, 0, rapidright32WidthPixels, rapidright32HeightPixels);
	}

	void Display::DrawSpeed(uint32_t aSpeed)
	{
		char speed[13];
		snprintf(speed, sizeof(speed), "%u", aSpeed);
		if (myUnits == Units::Millimeter)
		{

			strcat(speed, " ");
			strcat(speed, MMPM);
		}
		else
		{
			snprintf(speed, sizeof(speed), "%u", aSpeed);
			strcat(speed, " ");
			strcat(speed, IPM);
		}
		DrawCenteredText(speed, font_12x16, 0);
	}

	void Display::DrawCenteredText(const char *text, const unsigned char *font, uint16_t y)
	{
		uint16_t textWidth = GetTextWidth(text, font);
		uint16_t x = (myWidth - textWidth) / 2;
		DrawText(text, font, x, y);
	}

	uint16_t Display::GetTextWidth(const char *text, const unsigned char *font)
	{
		return font[0] * strlen(text);
	}

} // namespace PicoMill