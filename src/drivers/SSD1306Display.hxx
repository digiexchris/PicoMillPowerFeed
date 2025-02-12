#pragma once

#include "Display.hxx"
#include "Settings.hxx"
#include "ssd1306.h"
#include <memory>
#include <sys/_stdint.h>

namespace PowerFeed::Drivers
{

	class SSD1306Display : public Display
	{
	public:
		SSD1306Display(std::shared_ptr<SettingsManager> settings);
		void DrawText(const char *text, const unsigned char *font, uint16_t x, uint16_t y) override;
		void DrawImage(const unsigned char *image, uint16_t x, uint16_t y, uint16_t width, uint16_t height) override;
		void ClearBuffer() override;
		void WriteBuffer() override;
		void Refresh() override;

	private:
		void WaitForInit();
		pico_ssd1306::SSD1306 *mySSD1306;
		uint64_t myInitTime = 0;
		bool myIsReady = false;
	};

}