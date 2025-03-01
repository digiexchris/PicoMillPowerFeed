#include "Display.hxx"
#include "Helpers.hxx"
#include "Settings.hxx"
#include "icons.hxx"
#include <cstring>

namespace PowerFeed
{

	const char *IPM = "ipm";
	const char *MMPM = "mm ";

	Display::Display(SettingsManager *settings, const unsigned char *font) : mySettings(settings), myFont(font)
	{
	}

	void Display::DrawStart()
	{
		ClearBuffer();

		const char *top = "POWER";
		DrawCenteredText(top, myFont, 16);

		const char *bottom = "FEED";
		DrawCenteredText(bottom, myFont, 32);
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
		float speedPerMin = (static_cast<float>(aSpeed) / mySettings->Get()->mechanical.stepsPerMm) * 60.0;
		char speed[14];

		if (myUnits == Units::Millimeter)
		{
			snprintf(speed, sizeof(speed), "%.1f %s", speedPerMin, MMPM);
		}
		else
		{
			speedPerMin = speedPerMin * inchPerMm;
			snprintf(speed, sizeof(speed), "%.1f %s", speedPerMin, IPM);
		}
		DrawCenteredText(speed, myFont, 0);
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

} // namespace PowerFeed