#include "SSD1306Display.hxx"
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

	SSD1306Display::SSD1306Display()
	{
		// Initialize the display

		// Init i2c0 controller
		i2c_init(I2C_MASTER_NUM, 100000);
		// Set up pins 12 and 13
		gpio_set_function(I2C_MASTER_SDA_IO, GPIO_FUNC_I2C);
		gpio_set_function(I2C_MASTER_SCL_IO, GPIO_FUNC_I2C);
		gpio_pull_up(I2C_MASTER_SDA_IO);
		gpio_pull_up(I2C_MASTER_SCL_IO);

		// If you don't do anything before initializing a display pi pico is too fast and starts sending
		// commands before the screen controller had time to set itself up, so we add an artificial delay for
		// ssd1306 to set itself up. This gets checked before each write to see if enough time has passed.
		sleep_us(250);

		// Create a new display object at address 0x3D and size of 128x64
		mySSD1306 = new pico_ssd1306::SSD1306(pico_ssd1306::SSD1306(I2C_MASTER_NUM, SSD1306_ADDRESS, pico_ssd1306::Size::W128xH64));

		// Here we rotate the display by 180 degrees, so that it's not upside down from my perspective
		// If your screen is upside down try setting it to 1 or 0

#if SSD1306_ROTATE_180_DEGREES
		myDismySSD1306play->setOrientation(1);
#endif

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