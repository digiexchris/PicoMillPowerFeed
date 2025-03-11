#pragma once

#include "Settings.hxx"
#include <cstdint>
#include <hardware/flash.h>
#include <hardware/regs/addressmap.h>
#include <hardware/sync.h>
#include <lfs.h>
#include <memory>

#include <hardware/flash.h>

// Calculate safe storage address directly using the Pico SDK's constants
#define FS_SIZE (256 * 1024) // 256KB filesystem size
#define CONFIG_FILENAME "/config.json"

// Define our filesystem partition at a safe location
// Use PICO_FLASH_SIZE_BYTES from the SDK, and place filesystem at the top of flash
// with a safety margin to avoid any potential overlap with the program or bootloader
#define FILESYSTEM_OFFSET (PICO_FLASH_SIZE_BYTES - FS_SIZE - FLASH_SECTOR_SIZE)
#define FILESYSTEM_ADDR (XIP_BASE + FILESYSTEM_OFFSET)

// Forward declaration for FreeRTOS timer
extern "C" {
#include "FreeRTOS.h"
#include "timers.h"
}

namespace PowerFeed::Drivers
{
	class LittleFSSettings : public SettingsManager
	{
	public:
		LittleFSSettings();
		~LittleFSSettings();
		
		// Using ScheduleAutoSave from base class SettingsManager
		
		/**
		 * @brief Save settings immediately with explicit settings object
		 */
		void SaveNow(std::shared_ptr<Settings> aSettings);
		
		/**
		 * @brief Save current settings immediately without waiting for the timer
		 * Implementation of the base class method
		 */
		void SaveNow() override;

		/**
		 * @brief Read from LittleFS and return settings, create default if doesn't exist
		 */
		std::shared_ptr<Settings> Load() override;

		/**
		 * @brief Save settings to LittleFS
		 */
		void Save(std::shared_ptr<Settings> aSettings) override;

		/**
		 * @brief Return previously retrieved settings
		 */
		std::shared_ptr<Settings> Get() override;

		/**
		 * @brief Initialize the filesystem
		 */
		bool InitializeFS();

		/**
		 * @brief Check if a file exists in the filesystem
		 */
		bool FileExists(const char *aPath);

		/**
		 * @brief Create a file in the filesystem with the given content
		 */
		bool CreateFile(const char *aPath, const char *aContent);

		/**
		 * @brief Read a file from the filesystem
		 */
		bool ReadFile(const char *aPath, char *aBuffer, size_t aBufferSize);

		/**
		 * @brief Get the LittleFS instance
		 */
		lfs_t *GetFS() { return &myFs; }

	private:
		std::shared_ptr<Settings> mySettings;
		lfs_t myFs;
		bool myFsMounted;

		/**
		 * @brief Check FS and format if necessary
		 */
		void CheckFS();
		
		// LittleFS flash functions
		static int BlockDeviceRead(const struct lfs_config *c,
								   lfs_block_t block,
								   lfs_off_t off,
								   void *buffer,
								   lfs_size_t size);

		static int BlockDeviceWrite(const struct lfs_config *c,
									lfs_block_t block,
									lfs_off_t off,
									const void *buffer,
									lfs_size_t size);

		static int BlockDeviceErase(const struct lfs_config *c, lfs_block_t block);

		static int BlockDeviceSync(const struct lfs_config *c);

		static uint32_t GetFsBaseAddress(const struct lfs_config *c);

		// LittleFS configuration
		struct lfs_config myFlashConfig;
	};
}