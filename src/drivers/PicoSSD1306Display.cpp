#include "PicoSSD1306Display.hpp"
#include "hardware/i2c.h"
#include "ssd1306.h"
#include "textRenderer/12x16_font.h"
#include "textRenderer/TextRenderer.h"
#include <cstdint>

#include "Helpers.hpp"
#include "config.hpp"
#include <hardware/gpio.h>
#include <hardware/timer.h>
#include <memory>
#include <pico/time.h>

namespace PicoMill::Drivers
{

	PicoSSD1306Display::PicoSSD1306Display()
	{
		// Initialize the display

		// Init i2c0 controller
		i2c_init(I2C_MASTER_NUM, 100000);
		// Set up pins 12 and 13
		gpio_set_function(I2C_MASTER_SDA_IO, GPIO_FUNC_I2C);
		gpio_set_function(I2C_MASTER_SCL_IO, GPIO_FUNC_I2C);
		gpio_pull_up(I2C_MASTER_SDA_IO);
		gpio_pull_up(I2C_MASTER_SCL_IO);

		sleep_us(250);

		// If you don't do anything before initializing a display pi pico is too fast and starts sending
		// commands before the screen controller had time to set itself up, so we add an artificial delay for
		// ssd1306 to set itself up. This gets checked before each write to see if enough time has passed.
		myInitTime = time_us_64();

		auto display = pico_ssd1306::SSD1306(I2C_MASTER_NUM, SSD1306_ADDRESS, pico_ssd1306::Size::W128xH32);

		// Create a new display object at address 0x3D and size of 128x32
		myDisplay = std::make_unique<pico_ssd1306::SSD1306>(display);

		// Here we rotate the display by 180 degrees, so that it's not upside down from my perspective
		// If your screen is upside down try setting it to 1 or 0
		// myDisplay->setOrientation(0);

		myDisplay->turnOn();
	}

	void PicoSSD1306Display::WaitForInit()
	{
		// if (!myIsReady)
		// {
		// 	if (time_us_64() - myInitTime < MS_TO_US(250))
		// 	{
		// 		// Sleep the remaining time until 250ms have passed since myInitTime
		// 		sleep_us(MS_TO_US(250) - (time_us_64() - myInitTime));
		// 	}
		// 	else
		// 	{
		// 		myIsReady = true;
		// 	}
		// }
	}

	void PicoSSD1306Display::DrawText(const char *text, const unsigned char *font, uint16_t x, uint16_t y)
	{
		WaitForInit();
		pico_ssd1306::drawText(myDisplay.get(), font, text, x, y);
		myDisplay->sendBuffer();
	}

	void PicoSSD1306Display::DrawImage(const unsigned char *image, uint16_t x, uint16_t y, uint16_t width, uint16_t height)
	{
		WaitForInit();
		myDisplay->addBitmapImage(x, y, width, height, const_cast<uint8_t *>(reinterpret_cast<const uint8_t *>(image)));
		myDisplay->sendBuffer();
	}

	void PicoSSD1306Display::Clear()
	{
		WaitForInit();
		myDisplay->clear();
		myDisplay->sendBuffer();
	}

} // namespace PicoMill