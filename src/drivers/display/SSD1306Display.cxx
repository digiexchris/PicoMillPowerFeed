#include "SSD1306Display.hxx"
#include "Settings.hxx"
#include "hardware/i2c.h"
#include "ssd1306.h"
#include "textRenderer/12x16_font.h"
#include "textRenderer/TextRenderer.h"
#include <cstdint>

#include "Helpers.hxx"
#include "config.h"
#include <hardware/gpio.h>
#include <hardware/timer.h>
#include <memory>
#include <pico/time.h>

namespace PowerFeed::Drivers
{

	SSD1306Display::SSD1306Display(SettingsManager *aSettings) : Display(aSettings, font_12x16)
	{
		// Initialize the display
		auto settings = aSettings->Get();
		if (settings == nullptr)
		{
			panic("SSD1306Display: Failed to load settings\n");
			return;
		}

		auto display = settings->display;

		i2c_inst_t *i2c_master = nullptr;
		if (display.i2cMasterNum == 0)
		{
			i2c_master = i2c0;
		}
		else if (display.i2cMasterNum == 1)
		{
			i2c_master = i2c1;
		}
		else
		{
			panic("SSD1306Display: Invalid i2c master number\n");
			return;
		}

		uint sda = display.i2cMasterSdaIo;
		uint scl = display.i2cMasterSclIo;

		if (sda + 1 != scl)
		{
			panic("SSD1306Display: SDA and SCL pins must be adjacent\n");
			return;
		}

		// Init i2c0 controller
		i2c_init(i2c_master, 100000);
		// Set up pins 12 and 13
		gpio_set_function(sda, GPIO_FUNC_I2C);
		gpio_set_function(scl, GPIO_FUNC_I2C);
		gpio_pull_up(sda);
		gpio_pull_up(scl);

		// If you don't do anything before initializing a display pi pico is too fast and starts sending
		// commands before the screen controller had time to set itself up, so we add an artificial delay for
		// ssd1306 to set itself up. This gets checked before each write to see if enough time has passed.
		sleep_us(250);

		// Create a new display object at address 0x3D and size of 128x64
		mySSD1306 = new pico_ssd1306::SSD1306(pico_ssd1306::SSD1306(i2c_master, display.ssd1306Address, pico_ssd1306::Size::W128xH64));

		// Here we rotate the display by 180 degrees, so that it's not upside down from my perspective
		// If your screen is upside down try setting it to 1 or 0

		if (display.ssd1306Rotate180)
		{
			mySSD1306->setOrientation(1);
		}

		mySSD1306->turnOn();
	}

	void SSD1306Display::WriteBuffer()
	{
		mySSD1306->sendBuffer();
	}

	void SSD1306Display::Refresh()
	{
		WriteBuffer();
	}

	void SSD1306Display::DrawText(const char *text, const unsigned char *font, uint16_t x, uint16_t y)
	{
		pico_ssd1306::drawText(mySSD1306, font, text, x, y);
	}

	void SSD1306Display::DrawImage(const unsigned char *image, uint16_t x, uint16_t y, uint16_t width, uint16_t height)
	{
		mySSD1306->addBitmapImage(x, y, width, height, const_cast<uint8_t *>(reinterpret_cast<const uint8_t *>(image)));
	}

	void SSD1306Display::ClearBuffer()
	{
		mySSD1306->clear();
	}

} // namespace PowerFeed