#include "Display.hxx"
#include "Helpers.hxx"
#include "config.hxx"
#include "icons.hxx"
// #include "textRenderer/12x16_font.h"
// #include <stdio.h>
#include <nuttx/nx/nx.h>
#include <stdio.h>
#include <string.h>

#include <nuttx/nx/nxfonts.h>

namespace PicoMill
{

	const char *IPM = "ipm";
	const char *MMPM = "mm ";

	NXHANDLE Display::myDefaultFontHandle = nxf_getfonthandle(FONTID_PIXEL_LCD_MACHINE);

	Display::Display()
	{
		if (!myDefaultFontHandle)
		{
			printf("nxhello_main: Failed to get font handle: %d\n", errno);
			abort();
		}
	}

	void Display::DrawStart()
	{
		ClearBuffer();

		const char *top = "POWER";
		DrawCenteredText(top, myDefaultFontHandle, 16);

		const char *bottom = "FEED";
		DrawCenteredText(bottom, myDefaultFontHandle, 32);
	}

	void Display::ToggleUnits()
	{
		myUnits = myUnits == Units::Millimeter ? Units::Inch : Units::Millimeter;
	}

	void Display::DrawMovingLeft()
	{
		DrawImage(moveleft32, leftX, 32, moveleft32WidthPixels, moveleft32HeightPixels);
	}

	void Display::DrawMovingRight()
	{
		DrawImage(moveright32, rightX, 32, moveright32WidthPixels, moveright32HeightPixels);
	}

	void Display::DrawStopping()
	{
		DrawImage(stop32, leftX, 16, stop32WidthPixels, stop32HeightPixels);
		DrawImage(stop32, rightX, 16, stop32WidthPixels, stop32HeightPixels);
	}

	void Display::DrawStopped()
	{
		DrawImage(stop32, leftX, 32, stop32WidthPixels, stop32HeightPixels);
		DrawImage(stop32, rightX, 32, stop32WidthPixels, stop32HeightPixels);
	}

	void Display::DrawRapidLeft()
	{
		DrawImage(rapidleft32, leftX, 32, rapidleft32WidthPixels, rapidleft32HeightPixels);
	}

	void Display::DrawRapidRight()
	{
		DrawImage(rapidright32, rightX, 32, rapidright32WidthPixels, rapidright32HeightPixels);
	}

	void Display::DrawSpeed(uint32_t aSpeed)
	{
		float speedPerMin = (static_cast<float>(aSpeed) / stepsPerMm) * 60.0;
		char speed[14];

		if (myUnits == Units::Millimeter)
		{
			snprintf(speed, sizeof(speed), "%.1f %s", speedPerMin, MMPM);
		}
		else
		{
			speedPerMin = MMToInch(speedPerMin);
			snprintf(speed, sizeof(speed), "%.1f %s", speedPerMin, IPM);
		}
		DrawCenteredText(speed, myDefaultFontHandle, 0);
	}

	void Display::DrawCenteredText(const char *text, const NXHANDLE &aFont, uint16_t y)
	{
		uint16_t textWidth = GetTextWidth(text, aFont);
		uint16_t x = (myWidth - textWidth) / 2;
		DrawText(text, aFont, x, y);
	}

	uint16_t Display::GetTextWidth(const char *text, const NXHANDLE &aFont)
	{
		auto font = nxf_getfontset(aFont);
		return font->mxwidth * strlen(text);
	}

} // namespace PicoMill