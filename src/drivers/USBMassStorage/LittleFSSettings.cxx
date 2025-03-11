#include "LittleFSSettings.hxx"
#include "Settings.hxx"
#include "drivers/stepper/PicoStepper.hxx"
#include <hardware/flash.h>
#include <hardware/sync.h>
#include <lfs.h>
#include <nlohmann/json.hpp>
#include <pico/stdlib.h>
#include <stdio.h>
#include <string>

// Flash storage is located at the end of flash, before the bootloader
#define FLASH_SECTOR_SIZE 4096			 // 65536
#define FLASH_BLOCK_SIZE FLASH_PAGE_SIZE // 256
// Use our safe offset defined in the header
// No longer using FLASH_TARGET_OFFSET = (PICO_FLASH_SIZE_BYTES - FS_SIZE)

namespace PowerFeed::Drivers
{
	LittleFSSettings::LittleFSSettings() : SettingsManager(), myFsMounted(false)
	{
		// Initialize default settings from the base class
		myDefaultSettings = GetDefaultSettings();

		// Configure LittleFS
		memset(&myFlashConfig, 0, sizeof(myFlashConfig));
		myFlashConfig.read = BlockDeviceRead;
		myFlashConfig.prog = BlockDeviceWrite;
		myFlashConfig.erase = BlockDeviceErase;
		myFlashConfig.sync = BlockDeviceSync;
		myFlashConfig.read_size = 16;
		myFlashConfig.prog_size = 16;
		myFlashConfig.block_size = FLASH_SECTOR_SIZE;
		myFlashConfig.block_count = FS_SIZE / FLASH_SECTOR_SIZE;
		myFlashConfig.cache_size = FLASH_BLOCK_SIZE;
		myFlashConfig.lookahead_size = 16;
		myFlashConfig.block_cycles = 500;

		// Initialize the filesystem
		InitializeFS();
		
		// Note: Auto-save timer is created by the base class constructor
	}

	LittleFSSettings::~LittleFSSettings()
	{
		// Note: Timer is deleted by base class destructor
		
		// Unmount the filesystem
		if (myFsMounted)
		{
			lfs_unmount(&myFs);
			myFsMounted = false;
		}
	}

	std::shared_ptr<Settings> LittleFSSettings::Load()
	{
		taskENTER_CRITICAL();
		if (!myFsMounted)
		{
			if (!InitializeFS())
			{
				printf("Failed to initialize filesystem\n");
				return myDefaultSettings;
			}
		}

		if (!FileExists(CONFIG_FILENAME))
		{
			printf("Config file not found, creating default\n");

			// Get the default config from the JSON file
			char buffer[4096];
			auto defaultConfig = myDefaultSettings->to_json();
			std::string configStr = defaultConfig.dump(2);

			// Create the config file
			if (!CreateFile(CONFIG_FILENAME, configStr.c_str()))
			{
				printf("Failed to create config file\n");
				return myDefaultSettings;
			}

			mySettings = myDefaultSettings;
			return mySettings;
		}

		// Read the configuration file
		char buffer[4096] = {0};
		if (!ReadFile(CONFIG_FILENAME, buffer, sizeof(buffer)))
		{
			printf("Failed to read config file\n");
			return myDefaultSettings;
		}

		taskEXIT_CRITICAL();

		try
		{
			// Parse JSON
			nlohmann::json jsonConfig = nlohmann::json::parse(buffer);
			mySettings = std::make_shared<Settings>(Settings::from_json(jsonConfig));
		}
		catch (const std::exception &e)
		{
			printf("Failed to parse config: %s\n", e.what());
			return myDefaultSettings;
		}

		return mySettings;
	}

	void LittleFSSettings::Save(std::shared_ptr<Settings> aSettings)
	{
		// Just call SaveNow with the provided settings
		SaveNow(aSettings);
	}

	void LittleFSSettings::SaveNow()
	{
		// Implementation of base class method - saves current settings
		SaveNow(Get());
	}

	void LittleFSSettings::SaveNow(std::shared_ptr<Settings> aSettings)
	{
		// We don't need to schedule an auto-save here since we're saving immediately
		
		if (!myFsMounted && !InitializeFS())
		{
			printf("Failed to initialize filesystem for saving\n");
			return;
		}

		// Convert settings to JSON
		nlohmann::json jsonConfig = aSettings->to_json();
		std::string configStr = jsonConfig.dump(2);

		// Create file (overwrite if exists)
		lfs_file_t file;
		int err = lfs_file_open(&myFs, &file, CONFIG_FILENAME, LFS_O_WRONLY | LFS_O_CREAT | LFS_O_TRUNC);
		if (err < 0)
		{
			printf("Failed to open file for saving: %d\n", err);
			return;
		}

		// Write configuration
		lfs_size_t written = lfs_file_write(&myFs, &file, configStr.c_str(), configStr.size());
		if (written != configStr.size())
		{
			printf("Failed to write complete file: %u/%u\n", written, configStr.size());
		}

		lfs_file_close(&myFs, &file);

		// Update cached settings
		mySettings = aSettings;
		printf("Settings saved to flash\n");
	}

	// ScheduleAutoSave method removed - using base class implementation
	// Timer callback functionality moved to SettingsManager base class

	std::shared_ptr<Settings> LittleFSSettings::Get()
	{
		if (mySettings == nullptr)
		{
			return Load();
		}
		return mySettings;
	}

	bool LittleFSSettings::InitializeFS()
	{
		if (myFsMounted)
		{
			return true;
		}

		// Try mounting the filesystem from our partition
		printf("Attempting to mount filesystem from offset 0x%x (XIP addr 0x%x)\n", FILESYSTEM_OFFSET, FILESYSTEM_ADDR);
		int err = lfs_mount(&myFs, &myFlashConfig);

		// If mounting fails, initialize the filesystem area
		if (err != LFS_ERR_OK)
		{
			printf("Failed to mount filesystem (error %d), initializing...\n", err);

			// Erase the filesystem area first
			uint32_t ints = save_and_disable_interrupts();
			printf("Erasing flash area at offset 0x%x, size %d bytes\n", FILESYSTEM_OFFSET, FS_SIZE);
			flash_range_erase(FILESYSTEM_OFFSET, FS_SIZE);
			restore_interrupts(ints);

			// Format the filesystem
			err = lfs_format(&myFs, &myFlashConfig);
			if (err != LFS_ERR_OK)
			{
				printf("Failed to format filesystem: %d\n", err);
				return false;
			}

			// Try mounting the newly formatted filesystem
			err = lfs_mount(&myFs, &myFlashConfig);
			if (err != LFS_ERR_OK)
			{
				printf("Failed to mount filesystem after format: %d\n", err);
				return false;
			}

			// Create the default config file
			printf("Creating default config file\n");
			auto defaultConfig = myDefaultSettings->to_json();
			std::string configStr = defaultConfig.dump(2);

			if (!CreateFile(CONFIG_FILENAME, configStr.c_str()))
			{
				printf("Failed to create default config file\n");
			}
		}
		else
		{
			printf("Successfully mounted existing filesystem\n");
		}

		myFsMounted = true;
		return true;
	}

	bool LittleFSSettings::FileExists(const char *aPath)
	{
		if (!myFsMounted)
		{
			return false;
		}

		struct lfs_info info;
		return lfs_stat(&myFs, aPath, &info) >= 0;
	}

	bool LittleFSSettings::CreateFile(const char *aPath, const char *aContent)
	{
		if (!myFsMounted)
		{
			return false;
		}

		lfs_file_t file;
		int err = lfs_file_open(&myFs, &file, aPath, LFS_O_WRONLY | LFS_O_CREAT | LFS_O_TRUNC);
		if (err < 0)
		{
			printf("Failed to create file: %d\n", err);
			return false;
		}

		size_t contentLength = strlen(aContent);
		lfs_size_t written = lfs_file_write(&myFs, &file, aContent, contentLength);

		lfs_file_close(&myFs, &file);

		return (written == contentLength);
	}

	bool LittleFSSettings::ReadFile(const char *aPath, char *aBuffer, size_t aBufferSize)
	{
		if (!myFsMounted)
		{
			return false;
		}

		lfs_file_t file;
		int err = lfs_file_open(&myFs, &file, aPath, LFS_O_RDONLY);
		if (err < 0)
		{
			printf("Failed to open file for reading: %d\n", err);
			return false;
		}

		lfs_size_t read = lfs_file_read(&myFs, &file, aBuffer, aBufferSize - 1);
		if (read < 0)
		{
			printf("Failed to read file: %d\n", read);
			lfs_file_close(&myFs, &file);
			return false;
		}

		// Null terminate
		aBuffer[read] = '\0';

		lfs_file_close(&myFs, &file);
		return true;
	}

	// LittleFS Flash IO functions
	int LittleFSSettings::BlockDeviceRead(const struct lfs_config *c, lfs_block_t block, lfs_off_t off, void *buffer, lfs_size_t size)
	{
		// Calculate flash address: base offset + block offset + byte offset
		uint32_t addr = GetFsBaseAddress(c) + (block * c->block_size) + off;
		// Access using XIP for reading
		memcpy(buffer, (void *)(XIP_BASE + addr), size);
		return LFS_ERR_OK;
	}

	int LittleFSSettings::BlockDeviceWrite(const struct lfs_config *c, lfs_block_t block, lfs_off_t off, const void *buffer, lfs_size_t size)
	{
		// Calculate flash address: base offset + block offset + byte offset
		uint32_t addr = GetFsBaseAddress(c) + (block * c->block_size) + off;

		// Disable interrupts during flash write
		uint32_t ints = save_and_disable_interrupts();

		// Use the SDK's flash program function
		flash_range_program(addr, (const uint8_t *)buffer, size);

		restore_interrupts(ints);
		return LFS_ERR_OK;
	}

	int LittleFSSettings::BlockDeviceErase(const struct lfs_config *c, lfs_block_t block)
	{
		// Calculate flash address for this block
		uint32_t addr = GetFsBaseAddress(c) + (block * c->block_size);

		// Disable interrupts during flash erase
		uint32_t ints = save_and_disable_interrupts();

		// Use the SDK's flash erase function
		flash_range_erase(addr, c->block_size);

		restore_interrupts(ints);
		return LFS_ERR_OK;
	}

	int LittleFSSettings::BlockDeviceSync(const struct lfs_config *c)
	{
		// Flash writes are synchronous, no action needed
		return LFS_ERR_OK;
	}

	uint32_t LittleFSSettings::GetFsBaseAddress(const struct lfs_config *c)
	{
		// Return the offset from the beginning of flash
		return FILESYSTEM_OFFSET;
	}
}