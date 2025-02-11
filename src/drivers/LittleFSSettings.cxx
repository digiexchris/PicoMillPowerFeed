#include "LittleFSSettings.hxx"
#include "hardware/flash.h"
#include "hardware/sync.h"

#include "fs/mimic_fat.h"
#include "portmacro.h"
#include <bsp/board.h>
#include <lfs.h>
#include <pico/stdlib.h>
#include <stdlib.h>
#include <string.h>
#include <tusb.h>
namespace PowerFeed::Drivers
{

#define FILENAME "CONFIG.JSON"

#define README_TXT                                                               \
	"POWER FEED\n"                                                               \
	"\n"                                                                         \
	"A power feed for milling machines or similar industrial machinery.\n"       \
	"See https://github.com/digiexchris/PicoMillPowerFeed/blob/main/config.md\n" \
	"for instructions on how to configure the device.\n"

#define ANSI_RED "\e[31m"
#define ANSI_CLEAR "\e[0m"

	/*
	 * Format the file system if it does not exist
	 */
	void LittleFSSettings::CheckFS()
	{
		if ((lfs_mount(&myFs, &myFlashConfig) != 0))
		{
			printf("Format the onboard flash memory with littlefs\n");

			lfs_format(&myFs, &myFlashConfig);
			lfs_mount(&myFs, &myFlashConfig);

			lfs_file_t file;
			lfs_file_open(&myFs, &file, "README.TXT", LFS_O_RDWR | LFS_O_CREAT);
			lfs_file_write(&myFs, &file, README_TXT, strlen(README_TXT));
			lfs_file_close(&myFs, &file);

			if (mimic_fat_usb_device_is_enabled())
			{
				mimic_fat_create_cache();
			}
		}
	}

	/*
	 * Log clicks on the BOOTSEL button to a log file.
	 * Press and hold the button for 10 seconds to initialize the file system.
	 */
	static void sensorLoggingTask(void)
	{
		// static bool lastStatus = false;
		// static int count = 0;
		// bool button = bb_get_bootsel_button();
		// static uint64_t longPush = 0;

		// if (lastStatus != button && button)
		// { // Push BOOTSEL button
		// 	count += 1;
		// 	printf("Update %s\n", FILENAME);

		// 	lfs_file_t file;
		// 	lfs_file_open(&myFs, &file, FILENAME, LFS_O_RDWR | LFS_O_APPEND | LFS_O_CREAT);
		// 	uint8_t buffer[512];
		// 	snprintf((char *)buffer, sizeof(buffer), "click=%d\n", count);
		// 	lfs_file_write(&myFs, &file, buffer, strlen((char *)buffer));
		// 	printf((char *)buffer);
		// 	lfs_file_close(&myFs, &file);
		// }
		// lastStatus = button;

		// if (button)
		// {
		// 	longPush++;
		// }
		// else
		// {
		// 	longPush = 0;
		// }
		// if (longPush > 35000)
		// { // Long-push BOOTSEL button
		// 	CheckFS();
		// 	count = 0;
		// 	longPush = 0;
		// }
	}

	// int main(void)
	// {
	// 	// set_sys_clock_khz(250000, false);

	// 	board_init();

	// 	stdio_init_all();

	// 	test_filesystem_and_format_if_necessary(false);
	// 	while (true)
	// 	{
	// 		sensorLoggingTask();
	// 		tud_task();
	// 	}
	// }

	LittleFSSettings::LittleFSSettings()
	{
		mySettings = std::make_shared<Settings>();

		tud_init(BOARD_TUD_RHPORT);
		xTaskCreate(TUDTask, "TUDTask", 2048, NULL, 12, NULL);
	}

	void LittleFSSettings::TUDTask(void *pvParameters)
	{
		while (true)
		{
			tud_task();
			vPortYield();
		}
	}

	std::shared_ptr<Settings> LittleFSSettings::Load()
	{

		return mySettings;
	}

	void LittleFSSettings::Save(std::shared_ptr<Settings> settings)
	{
		static bool lastStatus = false;
		static int count = 0;
		bool button = bb_get_bootsel_button();

		count += 1;
		printf("Update %s\n", FILENAME);

		lfs_file_t file;
		lfs_file_open(&myFs, &file, FILENAME, LFS_O_RDWR | LFS_O_APPEND | LFS_O_CREAT);
		uint8_t buffer[512];
		snprintf((char *)buffer, sizeof(buffer), "click=%d\n", count);
		lfs_file_write(&myFs, &file, buffer, strlen((char *)buffer));
		printf((char *)buffer);
		lfs_file_close(&myFs, &file);
	}
	lastStatus = button;
}
