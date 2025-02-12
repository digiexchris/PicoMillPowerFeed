#pragma once

#include "MachineState.hxx"
#include "Settings.hxx"
#include <cstdint>
#include <hardware/flash.h>
#include <hardware/regs/addressmap.h>
#include <hardware/sync.h>
#include <lfs.h>
#include <memory>

#define FS_SIZE (1.8 * 1024 * 1024)

namespace PowerFeed::Drivers
{

	class LittleFSSettings : public SettingsManager
	{
	public:
		LittleFSSettings();
		std::shared_ptr<Settings> Load() override;

		void Save(std::shared_ptr<Settings> settings) override;

	private:
		std::shared_ptr<Settings> mySettings;

		/**
		@brief Check FS and format if necessary
		 */
		void CheckFS();

		static void TUDTask(void *pvParameters);

		lfs_t myFs;

		static int erase(const struct lfs_config *c, lfs_block_t block)
		{
			(void)c;
			uint32_t off = block * FLASH_SECTOR_SIZE;
			uint32_t ints = save_and_disable_interrupts();
			flash_range_erase(fs_base(c) + off, FLASH_SECTOR_SIZE);
			restore_interrupts(ints);
			return 0;
		}

		static int read(const struct lfs_config *c,
						lfs_block_t block,
						lfs_off_t off,
						void *buffer,
						lfs_size_t size)
		{
			(void)c;

			uint8_t *p = (uint8_t *)(XIP_NOCACHE_NOALLOC_BASE + fs_base(c) + (block * FLASH_SECTOR_SIZE) + off);
			memcpy(buffer, p, size);
			return 0;
		}

		static int sync(const struct lfs_config *c)
		{
			(void)c;
			return 0;
		}

		static int prog(const struct lfs_config *c,
						lfs_block_t block,
						lfs_off_t off,
						const void *buffer,
						lfs_size_t size)
		{
			(void)c;
			uint32_t p = (block * FLASH_SECTOR_SIZE) + off;
			uint32_t ints = save_and_disable_interrupts();
			panic("Not implemented");
			// flash_range_program(fs_base(c) + p, buffer, size);
			restore_interrupts(ints);
			return 0;
		}

		static uint32_t fs_base(const struct lfs_config *c)
		{
			uint32_t storage_size = c->block_count * c->block_size;
			return PICO_FLASH_SIZE_BYTES - storage_size;
		}

		const struct lfs_config myFlashConfig = {
			.read = read,
			.prog = prog,
			.erase = erase,
			.sync = sync,
			.read_size = 1,
			.prog_size = FLASH_PAGE_SIZE,
			.block_size = FLASH_SECTOR_SIZE,
			.block_count = static_cast<lfs_size_t>(FS_SIZE / FLASH_SECTOR_SIZE),
			.block_cycles = 500,
			.cache_size = FLASH_SECTOR_SIZE,
			.lookahead_size = 16,
		};
	};

} // namespace
