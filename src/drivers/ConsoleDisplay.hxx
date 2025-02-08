#pragma once

#include "../Display.hxx"
// #include <memory.h>
#include <stdint.h>

namespace PowerFeed::Drivers
{

	class ConsoleDisplay : public Display
	{
	public:
		ConsoleDisplay();
		void DrawText(const char *text, const NXHANDLE &aFont, uint16_t x, uint16_t y) override;
		void DrawImage(const unsigned char *image, uint16_t x, uint16_t y, uint16_t width, uint16_t height) override;
		void ClearBuffer() override;
		void WriteBuffer() override;

	private:
		void WaitForInit();
		// pico_ssd1306::SSD1306 *mySSD1306;
		uint64_t myInitTime = 0;
		bool myIsReady = false;
	};

}