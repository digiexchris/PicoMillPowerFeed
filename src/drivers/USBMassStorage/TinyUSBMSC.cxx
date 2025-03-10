#include "TinyUSBMSC.hxx"
#include "hardware/watchdog.h"
#include <cstring>
#include <pico/stdlib.h>
#include <stdio.h>
#include <tusb.h>

// Defined in linker script to reserve flash regions
extern "C"
{
	extern const char __flash_binary_end;
	void tud_task(void);
}

// Define a FAT filesystem that exposes the LittleFS contents
#define BLOCK_COUNT (FS_SIZE / BLOCK_SIZE) // Number of logical blocks
#define BLOCK_SIZE 512					   // Block size in bytes (standard for FAT)
#define VOLUME_LABEL "PowerFeedCfg"		   // Volume name (8.3 format)

// Flash data buffer (for disk operations)
static uint8_t flash_cache[BLOCK_SIZE];

// Storage for the singleton instance
PowerFeed::Drivers::TinyUSBMSC *PowerFeed::Drivers::TinyUSBMSC::myInstance = nullptr;

namespace PowerFeed::Drivers
{
	TinyUSBMSC::TinyUSBMSC(std::shared_ptr<LittleFSSettings> aLittlefsSettings)
		: myLittleFsSettings(aLittlefsSettings),
		  myConfigUpdated(false)
	{
		myInstance = this;

		board_init();
		tusb_init();
	}

	TinyUSBMSC::~TinyUSBMSC()
	{
		if (myInstance == this)
		{
			myInstance = nullptr;
		}
	}

	void TinyUSBMSC::Task()
	{
		// Handle TinyUSB events
		tud_task();
	}

	void TinyUSBMSC::SetConfigUpdateCallback(std::function<void()> aCallback)
	{
		myConfigUpdateCallback = aCallback;
	}

	void TinyUSBMSC::UpdateConfigFromFS()
	{
		// Load the settings from the file
		std::shared_ptr<Settings> settings = myLittleFsSettings->Load();
		myConfigUpdated = false;
		if (myConfigUpdateCallback)
		{
			myConfigUpdateCallback();
		}
	}

	// --- TinyUSB MSC Callbacks ---

	// MSC read callback - transfer data from flash to USB host
	bool TinyUSBMSC::MSCReadCallback(uint32_t lba, void *buffer, uint32_t offset, uint32_t bufsize)
	{
		TinyUSBMSC *instance = GetInstance();
		if (!instance || !instance->myLittleFsSettings)
		{
			return false;
		}

		// Check if access is to the configuration file block
		if (lba == 0)
		{
			// FAT filesystem root directory simulation
			// Create a simple FAT structure pointing to our config file
			memset(buffer, 0, bufsize);

			// Basic FAT boot sector
			uint8_t *data = (uint8_t *)buffer;

			// Set boot signature
			data[510] = 0x55;
			data[511] = 0xAA;

			// Simple FAT structure (we don't implement full FAT - just make the file visible)
			strcpy((char *)data + 3, "PowerFeedFS"); // OEM name

			return true;
		}
		else if (lba == 1)
		{
			// FAT1 (File Allocation Table)
			memset(buffer, 0, bufsize);
			uint8_t *data = (uint8_t *)buffer;
			data[0] = 0xF8; // Media descriptor byte
			data[1] = 0xFF;
			data[2] = 0xFF;
			data[3] = 0xFF; // End of chain for config.json
			return true;
		}
		else if (lba == 2)
		{
			// FAT2 (Copy of File Allocation Table)
			memset(buffer, 0, bufsize);
			uint8_t *data = (uint8_t *)buffer;
			data[0] = 0xF8; // Media descriptor byte
			data[1] = 0xFF;
			data[2] = 0xFF;
			data[3] = 0xFF; // End of chain for config.json
			return true;
		}
		else if (lba == 3)
		{
			// Root directory
			memset(buffer, 0, bufsize);
			uint8_t *data = (uint8_t *)buffer;

			// Volume label entry
			memcpy(data, VOLUME_LABEL, strlen(VOLUME_LABEL));
			data[11] = 0x08; // Volume label attribute

			// CONFIG.JSON file entry (starting at offset 32 in directory)
			data += 32;
			memcpy(data, "CONFIG  JSON", 11); // 8.3 filename format
			data[11] = 0x20;				  // Archive attribute

			// Get file size if it exists
			lfs_t *fs = instance->myLittleFsSettings->GetFS();
			struct lfs_info info;
			uint32_t file_size = 0;

			if (lfs_stat(fs, CONFIG_FILENAME, &info) >= 0)
			{
				file_size = info.size;
			}

			// Set file size
			data[28] = file_size & 0xFF;
			data[29] = (file_size >> 8) & 0xFF;
			data[30] = (file_size >> 16) & 0xFF;
			data[31] = (file_size >> 24) & 0xFF;

			// Set cluster to 2 (first available cluster)
			data[26] = 2;
			data[27] = 0;

			return true;
		}
		else if (lba >= 4)
		{
			// Data area - contains our config.json file
			memset(buffer, 0, bufsize);

			// Calculate the offset in the config file
			uint32_t fileOffset = (lba - 4) * BLOCK_SIZE;

			// Read from LittleFS
			char temp_buffer[BLOCK_SIZE + 1] = {0};
			lfs_t *fs = instance->myLittleFsSettings->GetFS();

			lfs_file_t file;
			if (lfs_file_open(fs, &file, CONFIG_FILENAME, LFS_O_RDONLY) >= 0)
			{
				lfs_file_seek(fs, &file, fileOffset, LFS_SEEK_SET);
				lfs_ssize_t read_size = lfs_file_read(fs, &file, temp_buffer, BLOCK_SIZE);
				lfs_file_close(fs, &file);

				if (read_size > 0)
				{
					memcpy(buffer, temp_buffer, read_size);
				}
			}

			return true;
		}

		return true;
	}

	// MSC write callback - transfer data from USB host to flash
	bool TinyUSBMSC::MSCWriteCallback(uint32_t lba, uint8_t *buffer, uint32_t offset, uint32_t bufsize)
	{
		TinyUSBMSC *instance = GetInstance();
		if (!instance || !instance->myLittleFsSettings)
		{
			return false;
		}

		// We only care about writes to the data area (LBA >= 4) which contains our config file
		if (lba >= 4)
		{
			// Calculate the offset in the config file
			uint32_t fileOffset = (lba - 4) * BLOCK_SIZE;

			// Write to LittleFS
			lfs_t *fs = instance->myLittleFsSettings->GetFS();

			lfs_file_t file;
			int res = lfs_file_open(fs, &file, CONFIG_FILENAME, LFS_O_WRONLY | LFS_O_CREAT);
			if (res >= 0)
			{
				lfs_file_seek(fs, &file, fileOffset, LFS_SEEK_SET);
				lfs_file_write(fs, &file, buffer, bufsize);
				lfs_file_close(fs, &file);

				// Mark as updated
				instance->myConfigUpdated = true;
			}
		}

		return true;
	}

	// MSC flush callback - sync data to flash
	void TinyUSBMSC::MSCFlushCallback(void)
	{
		TinyUSBMSC *instance = GetInstance();
		if (!instance || !instance->myLittleFsSettings)
		{
			return;
		}

		// Check if config was updated
		if (instance->myConfigUpdated)
		{
			instance->UpdateConfigFromFS();
		}
	}
}

// TinyUSB MSC callbacks (must be in C linkage)
extern "C"
{
	bool tud_msc_test_unit_ready_cb(uint8_t lun)
	{
		(void)lun;
		return true;
	}

	void tud_msc_inquiry_cb(uint8_t lun, uint8_t vendor_id[8], uint8_t product_id[16], uint8_t product_rev[4])
	{
		(void)lun;

		const char vid[] = "PowerFeed";
		const char pid[] = "Config Storage";
		const char rev[] = "1.0";

		memcpy(vendor_id, vid, strlen(vid));
		memcpy(product_id, pid, strlen(pid));
		memcpy(product_rev, rev, strlen(rev));
	}

	bool tud_msc_is_writable_cb(uint8_t lun)
	{
		(void)lun;
		return true; // Writeable drive
	}

	uint32_t tud_msc_get_block_count_cb(uint8_t lun)
	{
		(void)lun;
		return BLOCK_COUNT;
	}

	uint16_t tud_msc_get_block_size_cb(uint8_t lun)
	{
		(void)lun;
		return BLOCK_SIZE;
	}

	void tud_msc_capacity_cb(uint8_t lun, uint32_t *block_count, uint16_t *block_size)
	{
		(void)lun;
		*block_count = BLOCK_COUNT;
		*block_size = BLOCK_SIZE;
	}

	bool tud_msc_start_stop_cb(uint8_t lun, uint8_t power_condition, bool start, bool load_eject)
	{
		(void)lun;
		(void)power_condition;

		if (load_eject)
		{
			if (!start)
			{
				// Eject command from host (load_eject=true, start=false)
				PowerFeed::Drivers::TinyUSBMSC *instance = PowerFeed::Drivers::TinyUSBMSC::GetInstance();
				if (instance)
				{
					if (instance->myEjectedCallback)
					{
						instance->myEjectedCallback();
					}
				}
			}
			// else: Load command from host (load_eject=true, start=true)
		}
		// else: Power management command (load_eject=false)

		return true;
	}

	int32_t tud_msc_read10_cb(uint8_t lun, uint32_t lba, uint32_t offset, void *buffer, uint32_t bufsize)
	{
		(void)lun;
		return PowerFeed::Drivers::TinyUSBMSC::MSCReadCallback(lba, buffer, offset, bufsize) ? bufsize : 0;
	}

	int32_t tud_msc_write10_cb(uint8_t lun, uint32_t lba, uint32_t offset, uint8_t *buffer, uint32_t bufsize)
	{
		(void)lun;
		return PowerFeed::Drivers::TinyUSBMSC::MSCWriteCallback(lba, buffer, offset, bufsize) ? bufsize : 0;
	}

	void tud_msc_flush_cb(uint8_t lun)
	{
		(void)lun;
		PowerFeed::Drivers::TinyUSBMSC::MSCFlushCallback();
	}
}