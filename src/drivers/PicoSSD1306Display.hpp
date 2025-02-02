#pragma once

#include "Display.hpp"
#include "ssd1306.h"
#include <memory>
#include <sys/_stdint.h>

namespace PicoMill::Drivers
{

	class PicoSSD1306Display : public Display
	{
	public:
		PicoSSD1306Display();
		void DrawText(const char *text, const unsigned char *font, uint16_t x, uint16_t y) override;
		void DrawImage(const unsigned char *image, uint16_t x, uint16_t y, uint16_t width, uint16_t height) override;
		void ClearBuffer() override;
		void WriteBuffer() override;

	private:
		void WaitForInit();
		pico_ssd1306::SSD1306 *mySSD1306;
		uint64_t myInitTime = 0;
		bool myIsReady = false;
	};

}