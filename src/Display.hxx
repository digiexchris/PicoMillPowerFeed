#pragma once

#include "Helpers.hxx"
// #include "StepperState.hpp"
#include "config.hxx"
#include "icons.hxx"

#include <cstdint>
#include <nuttx/nx/nx.h>
#include <nuttx/nx/nxfonts.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

namespace PowerFeed
{

	static NXHANDLE DefaultFontHandle = nxf_getfonthandle(FONTID_PIXEL_LCD_MACHINE);

	enum class Units // per minute
	{
		Inch,
		Millimeter
	};

	template <typename T>
	class Display
	{
	public:
		// Display();
		// void DrawStart();
		// void DrawMovingLeft();
		// void DrawMovingRight();
		// void DrawStopping();
		// void DrawStopped();
		// void DrawRapidLeft();
		// void DrawRapidRight();
		// void DrawSpeed(uint32_t speed);
		// void ClearBuffer();
		// void WriteBuffer();
		// void ToggleUnits();

		Display()
		{
			T();
		}

		void Init()
		{
			static_cast<T *>(this)->Init();
		}

		void DrawStart()
		{
			static_cast<T *>(this)->ClearBuffer();

			const char *top = "POWER";
			DrawCenteredText(top, DefaultFontHandle, 16);

			const char *bottom = "FEED";
			DrawCenteredText(bottom, DefaultFontHandle, 32);
		}

		void ToggleUnits()
		{
			myUnits = myUnits == Units::Millimeter ? Units::Inch : Units::Millimeter;
		}

		void DrawMovingLeft()
		{
			static_cast<T *>(this)->DrawImage(moveleft32, leftX, 32, moveleft32WidthPixels, moveleft32HeightPixels);
		}

		void DrawMovingRight()
		{
			static_cast<T *>(this)->DrawImage(moveright32, rightX, 32, moveright32WidthPixels, moveright32HeightPixels);
		}

		void DrawStopping()
		{
			static_cast<T *>(this)->DrawImage(stop32, leftX, 16, stop32WidthPixels, stop32HeightPixels);
			static_cast<T *>(this)->DrawImage(stop32, rightX, 16, stop32WidthPixels, stop32HeightPixels);
		}

		void DrawStopped()
		{
			static_cast<T *>(this)->DrawImage(stop32, leftX, 32, stop32WidthPixels, stop32HeightPixels);
			static_cast<T *>(this)->DrawImage(stop32, rightX, 32, stop32WidthPixels, stop32HeightPixels);
		}

		void DrawRapidLeft()
		{
			static_cast<T *>(this)->DrawImage(rapidleft32, leftX, 32, rapidleft32WidthPixels, rapidleft32HeightPixels);
		}

		void DrawRapidRight()
		{
			static_cast<T *>(this)->DrawImage(rapidright32, rightX, 32, rapidright32WidthPixels, rapidright32HeightPixels);
		}

		void DrawSpeed(uint32_t aSpeed)
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
			DrawCenteredText(speed, DefaultFontHandle, 0);
		}

		void ClearBuffer()
		{
			static_cast<T *>(this)->ClearBuffer();
		}

		void WriteBuffer()
		{
			static_cast<T *>(this)->WriteBuffer();
		}

	protected:
		void DrawCenteredText(const char *text, const NXHANDLE &aFont, uint16_t y)
		{
			uint16_t textWidth = GetTextWidth(text, aFont);
			uint16_t x = (myWidth - textWidth) / 2;
			static_cast<T *>(this)->DrawText(text, aFont, x, y);
		}

		uint16_t GetTextWidth(const char *text, const NXHANDLE &aFont)
		{
			auto font = nxf_getfontset(aFont);
			return font->mxwidth * strlen(text);
		}

		void DrawText(const char *text, const NXHANDLE &aFont, uint16_t x, uint16_t y)
		{
			static_cast<T *>(this)->DrawText(text, aFont, x, y);
		}

		void DrawImage(const unsigned char *image, uint16_t x, uint16_t y, uint16_t width, uint16_t height)
		{
			static_cast<T *>(this)->DrawImage(image, x, y, width, height);
		}

		const char *IPM = "ipm";
		const char *MMPM = "mm ";

		// static NXHANDLE myBgHandle;
		// static NXHANDLE myNxHandle;

	private:
		const uint16_t myWidth = 128;
		const uint16_t myHeight = 64;
		Units myUnits = Units::Millimeter;

		// const uint8_t height = 132;
		// const uint8_t width = 64;
		const uint8_t leftX = 0;
		const uint8_t rightX = 128 - 32;
		const uint8_t topY = 0;
		const uint8_t bottomY = 0;
		const uint8_t middleX = 64 - 16;
	};

} // namespace PowerFeed