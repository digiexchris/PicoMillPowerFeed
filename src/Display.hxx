#pragma once

#include "Settings.hxx"
#include <cstdint>
#include <string>

namespace PowerFeed
{

	enum class Units // per minute
	{
		Inch,
		Millimeter
	};

	class Display
	{
	public:
		Display(SettingsManager *settings, const unsigned char *font);
		virtual void DrawStart();
		virtual void DrawMovingLeft();
		virtual void DrawMovingRight();
		virtual void DrawStopping();
		virtual void DrawStopped();
		virtual void DrawRapidLeft();
		virtual void DrawRapidRight();
		virtual void DrawSpeed(uint32_t aSpeed);
		virtual void ClearBuffer() = 0;
		virtual void ToggleUnits();
		virtual Units GetUnits() const { return myUnits; }
		virtual void WriteBuffer() = 0;
		virtual void Refresh() = 0;
		virtual void SetUnits(Units units) { myUnits = units; }

		bool IsReady() const { return myIsReady; }

	protected:
		virtual void DrawCenteredText(const char *text, const unsigned char *font, uint16_t y);
		virtual void DrawText(const char *text, const unsigned char *font, uint16_t x, uint16_t y) = 0;
		virtual void DrawImage(const unsigned char *image, uint16_t x, uint16_t y, uint16_t width, uint16_t height) = 0;

		uint16_t GetTextWidth(const char *text, const unsigned char *font);

	protected:
		Units myUnits = Units::Millimeter;
		bool myIsReady = false;

	private:
		const uint16_t myWidth = 128;
		const uint16_t myHeight = 32;
		uint32_t mySpeed = 0;

		const float mmPerInch = 25.4;
		const float inchPerMm = 1.0 / mmPerInch;

		const uint8_t height = 32;
		const uint8_t width = 32;
		const uint8_t leftX = 0;
		const uint8_t rightX = 128 - 32;
		const uint8_t topY = 0;
		const uint8_t bottomY = 0;
		const uint8_t middleX = 64 - 16;
		const unsigned char *myFont;

		SettingsManager *mySettings;
	};

} // namespace PowerFeed