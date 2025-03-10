#include "LittleFSSettings.hxx"
#include "Settings.hxx"
#include <hardware/flash.h>
#include <hardware/sync.h>
#include <lfs.h>
#include <nlohmann/json.hpp>
#include <pico/stdlib.h>
#include <stdio.h>
#include <string>

// Flash storage is located at the end of flash, before the bootloader
// #define FLASH_SECTOR_SIZE FLASH_SECTOR_SIZE
// #define FLASH_BLOCK_SIZE FLASH_PAGE_SIZE
#define FLASH_TARGET_OFFSET (PICO_FLASH_SIZE_BYTES - FS_SIZE)

namespace PowerFeed::Drivers
{
	LittleFSSettings::LittleFSSettings() : myFsMounted(false)
	{
		// Initialize default settings
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
	}

	LittleFSSettings::~LittleFSSettings()
	{
		if (myFsMounted)
		{
			lfs_unmount(&myFs);
			myFsMounted = false;
		}
	}

	std::shared_ptr<Settings> LittleFSSettings::Load()
	{
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
	}

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

		int err = lfs_mount(&myFs, &myFlashConfig);
		if (err != LFS_ERR_OK)
		{
			printf("Failed to mount filesystem (error %d), formatting...\n", err);

			// Erase filesystem
			err = lfs_format(&myFs, &myFlashConfig);
			if (err != LFS_ERR_OK)
			{
				printf("Failed to format filesystem: %d\n", err);
				return false;
			}

			// Try mounting again after format
			err = lfs_mount(&myFs, &myFlashConfig);
			if (err != LFS_ERR_OK)
			{
				printf("Failed to mount filesystem after format: %d\n", err);
				return false;
			}
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
		uint32_t addr = GetFsBaseAddress(c) + (block * c->block_size) + off;
		memcpy(buffer, (void *)(XIP_BASE + addr), size);
		return LFS_ERR_OK;
	}

	int LittleFSSettings::BlockDeviceWrite(const struct lfs_config *c, lfs_block_t block, lfs_off_t off, const void *buffer, lfs_size_t size)
	{
		uint32_t addr = GetFsBaseAddress(c) + (block * c->block_size) + off;
		uint32_t ints = save_and_disable_interrupts();

		flash_range_program(addr, (const uint8_t *)buffer, size);

		restore_interrupts(ints);
		return LFS_ERR_OK;
	}

	int LittleFSSettings::BlockDeviceErase(const struct lfs_config *c, lfs_block_t block)
	{
		uint32_t addr = GetFsBaseAddress(c) + (block * c->block_size);
		uint32_t ints = save_and_disable_interrupts();

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
		return FLASH_TARGET_OFFSET;
	}
}