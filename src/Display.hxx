#pragma once

#include "Settings.hxx"
#include <cstdint>
#include <memory>
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
		Display(std::shared_ptr<SettingsManager> settings);
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
		virtual void WriteBuffer() = 0;
		virtual void Refresh() = 0;

	protected:
		virtual void DrawCenteredText(const char *text, const unsigned char *font, uint16_t y);
		virtual void DrawText(const char *text, const unsigned char *font, uint16_t x, uint16_t y) = 0;
		virtual void DrawImage(const unsigned char *image, uint16_t x, uint16_t y, uint16_t width, uint16_t height) = 0;

		uint16_t GetTextWidth(const char *text, const unsigned char *font);

	private:
		const uint16_t myWidth = 128;
		const uint16_t myHeight = 32;
		Units myUnits = Units::Millimeter;
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

		std::shared_ptr<SettingsManager> mySettings;
	};

} // namespace PowerFeed