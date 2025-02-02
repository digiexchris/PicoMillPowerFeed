// #include "SDIOSettings.hpp"
// #include "hardware/flash.h"
// #include "hardware/sync.h"

// #include "f_util.h"
// #include "ff.h"
// #include "hw_config.h"

// namespace PicoMill
// {

// 	// /**
// 	//  * @brief Get the number of SD cards.
// 	//  *
// 	//  * @return The number of SD cards, which is 1 in this case.
// 	//  */
// 	// size_t sd_get_num() { return 1; }

// 	// /**
// 	//  * @brief Get a pointer to an SD card object by its number.
// 	//  *
// 	//  * @param[in] num The number of the SD card to get.
// 	//  *
// 	//  * @return A pointer to the SD card object, or @c NULL if the number is invalid.
// 	//  */
// 	// sd_card_t *sd_get_by_num(size_t num)
// 	// {
// 	// 	if (0 == num)
// 	// 	{
// 	// 		// The number 0 is a valid SD card number.
// 	// 		// Return a pointer to the sd_card object.
// 	// 		return &sd_card;
// 	// 	}
// 	// 	else
// 	// 	{
// 	// 		// The number is invalid. Return @c NULL.
// 	// 		return NULL;
// 	// 	}
// 	// }

// 	SDIOSettings::SDIOSettings()
// 	{
// 		mySettings = std::make_shared<Settings>();
// 	}

// 	std::shared_ptr<Settings> SDIOSettings::Load()
// 	{
// 		// // Read from Flash memory
// 		// uint8_t buf[sizeof(Settings)];
// 		// const uint FLASH_TARGET_OFFSET = 0x1000;

// 		// flash_range_read(FLASH_TARGET_OFFSET, buf, sizeof(Settings));
// 		// memcpy(mySettings.get(), buf, sizeof(Settings));

// 		return mySettings;
// 	}

// 	void SDIOSettings::Save(std::shared_ptr<Settings> settings)
// 	{
// 		// const uint FLASH_TARGET_OFFSET = 0x1000;
// 		// const uint32_t sectorSize = FLASH_SECTOR_SIZE;

// 		// // Buffer must be FLASH_PAGE_SIZE aligned
// 		// uint8_t buf[sectorSize];
// 		// memset(buf, 0xFF, sectorSize);
// 		// memcpy(buf, settings.get(), sizeof(Settings));

// 		// // Note: flash_range_erase and program need interrupts disabled
// 		// uint32_t ints = save_and_disable_interrupts();
// 		// flash_range_erase(FLASH_TARGET_OFFSET, FLASH_SECTOR_SIZE);
// 		// flash_range_program(FLASH_TARGET_OFFSET, buf, FLASH_SECTOR_SIZE);
// 		// restore_interrupts(ints);

// 		// mySettings = settings;

// 		FRESULT fr = f_mount(&myFatFs, "", 1);
// 		if (FR_OK != fr)
// 		{
// 			panic("f_mount error: %s (%d)\n", FRESULT_str(fr), fr);
// 			// -1;
// 		}

// 		FIL fil;
// 		const char *const filename = "filename.txt";
// 		fr = f_open(&fil, filename, FA_OPEN_APPEND | FA_WRITE);
// 		if (FR_OK != fr && FR_EXIST != fr)
// 		{
// 			panic("f_open(%s) error: %s (%d)\n", filename, FRESULT_str(fr), fr);
// 			// return -1;
// 		}

// 		if (f_printf(&fil, "Hello, world!\n") < 0)
// 		{
// 			printf("f_printf failed\n");
// 		}

// 		fr = f_close(&fil);
// 		if (FR_OK != fr)
// 		{
// 			printf("f_close error: %s (%d)\n", FRESULT_str(fr), fr);
// 		}

// 		f_unmount("");
// 	}
// }