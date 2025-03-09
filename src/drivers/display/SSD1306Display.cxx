#include "SSD1306Display.hxx"
#include "Assert.hxx"
#include "Common.hxx"
#include "Helpers.hxx"
#include "Settings.hxx"
#include "config.h"
#include "hardware/i2c.h"
#include "ssd1306.h"
#include "textRenderer/12x16_font.h"
#include "textRenderer/TextRenderer.h"
#include <cstdint>
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
			Panic("SSD1306Display: Failed to load settings\n");
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
			Panic("SSD1306Display: Invalid i2c master number\n");
			return;
		}

		uint sda = display.i2cMasterSdaIo;
		uint scl = display.i2cMasterSclIo;

		if (sda + 1 != scl)
		{
			Panic("SSD1306Display: SDA and SCL pins must be adjacent\n");
			return;
		}

		// Init i2c0 controller
		i2c_init(i2c_master, 100000);
		// Set up pins for I2C
		gpio_set_function(sda, GPIO_FUNC_I2C);
		gpio_set_function(scl, GPIO_FUNC_I2C);
		gpio_pull_up(sda);
		gpio_pull_up(scl);

		// If you don't do anything before initializing a display pi pico is too fast and starts sending
		// commands before the screen controller had time to set itself up, so we add an artificial delay
		sleep_us(250);
		
		// Check if display is present on I2C bus to avoid hanging
		uint8_t rxdata;
		int ret = i2c_read_timeout_us(i2c_master, display.ssd1306Address, &rxdata, 1, false, 10000);
		if (ret < 0) {
			printf("SSD1306Display: Display not detected on I2C bus, address 0x%02x\n", display.ssd1306Address);
			// Use a flag to indicate display is not available
			myIsReady = false;
			return;
		}
		myIsReady = true;

		// Create a new display object at address and size of 128x64
		// Fix the double construction issue by creating the object directly
		mySSD1306 = new pico_ssd1306::SSD1306(i2c_master, display.ssd1306Address, pico_ssd1306::Size::W128xH64);

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
		if (!myIsReady) {
			return;
		}
		mySSD1306->sendBuffer();
	}

	void SSD1306Display::Refresh()
	{
		WriteBuffer();
	}

	void SSD1306Display::DrawText(const char *text, const unsigned char *font, uint16_t x, uint16_t y)
	{
		if (!myIsReady) {
			return;
		}
		pico_ssd1306::drawText(mySSD1306, font, text, x, y);
	}

	void SSD1306Display::DrawImage(const unsigned char *image, uint16_t x, uint16_t y, uint16_t width, uint16_t height)
	{
		if (!myIsReady) {
			return;
		}
		mySSD1306->addBitmapImage(x, y, width, height, const_cast<uint8_t *>(reinterpret_cast<const uint8_t *>(image)));
	}

	void SSD1306Display::ClearBuffer()
	{
		if (!myIsReady) {
			return;
		}
		mySSD1306->clear();
	}

} // namespace PowerFeed