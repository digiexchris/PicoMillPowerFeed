#include "PicoFlashSettings.hpp"
#include "hardware/flash.h"
#include "hardware/sync.h"

namespace PicoMill
{

	PicoFlashSettings::PicoFlashSettings()
	{
		mySettings = std::make_shared<Settings>();
	}

	std::shared_ptr<Settings> PicoFlashSettings::Load()
	{
		// Read from Flash memory
		uint8_t buf[sizeof(Settings)];
		const uint FLASH_TARGET_OFFSET = 0x1000;

		flash_range_read(FLASH_TARGET_OFFSET, buf, sizeof(Settings));
		memcpy(mySettings.get(), buf, sizeof(Settings));

		return mySettings;
	}

	void PicoFlashSettings::Save(std::shared_ptr<Settings> settings)
	{
		const uint FLASH_TARGET_OFFSET = 0x1000;
		const uint32_t sectorSize = FLASH_SECTOR_SIZE;

		// Buffer must be FLASH_PAGE_SIZE aligned
		uint8_t buf[sectorSize];
		memset(buf, 0xFF, sectorSize);
		memcpy(buf, settings.get(), sizeof(Settings));

		// Note: flash_range_erase and program need interrupts disabled
		uint32_t ints = save_and_disable_interrupts();
		flash_range_erase(FLASH_TARGET_OFFSET, FLASH_SECTOR_SIZE);
		flash_range_program(FLASH_TARGET_OFFSET, buf, FLASH_SECTOR_SIZE);
		restore_interrupts(ints);

		mySettings = settings;
	}
}