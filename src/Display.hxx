#pragma once

#include <nuttx/nx/nx.h>
#include <nuttx/nx/nxfonts.h>
#include <stdint.h>

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
		Display() noexcept;
		void DrawStart();
		void DrawMovingLeft();
		void DrawMovingRight();
		void DrawStopping();
		void DrawStopped();
		void DrawRapidLeft();
		void DrawRapidRight();
		void DrawSpeed(uint32_t speed);
		virtual void ClearBuffer() = 0;
		virtual void WriteBuffer() = 0;
		void ToggleUnits();

	protected:
		void DrawCenteredText(const char *text, const NXHANDLE &aFont, uint16_t y);
		virtual void DrawText(const char *text, const NXHANDLE &aFont, uint16_t x, uint16_t y) = 0;
		virtual void DrawImage(const unsigned char *image, uint16_t x, uint16_t y, uint16_t width, uint16_t height) = 0;

		uint16_t GetTextWidth(const char *text, const NXHANDLE &aFont);

		static NXHANDLE myDefaultFontHandle;
		static NXHANDLE myBgHandle;
		static NXHANDLE myNxHandle;

	private:
		const uint16_t myWidth = 128;
		const uint16_t myHeight = 32;
		Units myUnits = Units::Millimeter;

		const uint8_t height = 32;
		const uint8_t width = 32;
		const uint8_t leftX = 0;
		const uint8_t rightX = 128 - 32;
		const uint8_t topY = 0;
		const uint8_t bottomY = 0;
		const uint8_t middleX = 64 - 16;
	};

} // namespace PowerFeed