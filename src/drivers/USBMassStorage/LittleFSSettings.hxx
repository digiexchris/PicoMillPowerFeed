#pragma once

#include "Settings.hxx"
#include <cstdint>
#include <hardware/flash.h>
#include <hardware/regs/addressmap.h>
#include <hardware/sync.h>
#include <lfs.h>
#include <memory>

#define FS_SIZE (1.8 * 1024 * 1024)
#define CONFIG_FILENAME "/config.json"

namespace PowerFeed::Drivers
{
	class LittleFSSettings : public SettingsManager
	{
	public:
		LittleFSSettings();
		~LittleFSSettings();

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