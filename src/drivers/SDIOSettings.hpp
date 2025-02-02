#pragma once

// #include "MachineState.hpp"
// #include "Settings.hpp"
// #include "sd_card.h"
// #include <cstdint>
// #include <memory>

// namespace PicoMill
// {

// 	class SDIOSettings : public SettingsManager
// 	{
// 	public:
// 		SDIOSettings();
// 		std::shared_ptr<Settings> Load() override;

// 		void Save(std::shared_ptr<Settings> settings) override;

// 	private:
// 		std::shared_ptr<Settings> mySettings;
// 		sd_sdio_if_t mySdioIf = {
// 			/*
// 		Pins CLK_gpio, D1_gpio, D2_gpio, and D3_gpio are at offsets from pin D0_gpio.
// 		The offsets are determined by sd_driver\SDIO\rp2040_sdio.pio.
// 			CLK_gpio = (D0_gpio + SDIO_CLK_PIN_D0_OFFSET) % 32;
// 			As of this writing, SDIO_CLK_PIN_D0_OFFSET is 30,
// 				which is -2 in mod32 arithmetic, so:
// 			CLK_gpio = D0_gpio -2.
// 			D1_gpio = D0_gpio + 1;
// 			D2_gpio = D0_gpio + 2;
// 			D3_gpio = D0_gpio + 3;
// 		*/
// 			.CMD_gpio = 3,
// 			.D0_gpio = 4,
// 			.baud_rate = 125 * 1000 * 1000 / 6 // 20833333 Hz
// 		};

// 		sd_card_t mySdCard = {.type = SD_IF_SDIO, .sdio_if_p = &mySdioIf};

// 		FATFS myFatFs;
// 	};

// } // namespace PicoMill
