#pragma once

#include "LittleFSSettings.hxx"
#include <functional>
#include <memory>

// TinyUSB
extern "C"
{
#include "bsp/board.h"
#include "tusb.h"
}

namespace PowerFeed::Drivers
{
	class TinyUSBMSC
	{
	public:
		TinyUSBMSC(std::shared_ptr<LittleFSSettings> aLittlefsSettings);
		~TinyUSBMSC();

		/**
		 * @brief Service USB tasks, call regularly in main loop or task
		 */
		void Task();

		/**
		 * @brief Set the callback to be called when the config file is updated
		 * @param aCallback The callback function
		 */
		void SetConfigUpdateCallback(std::function<void()> aCallback);

		/**
		 * @brief Set the callback to be called when the USB drive is ejected
		 * @param aCallback The callback function
		 */
		void SetEjectedCallback(std::function<void()> aCallback);

		/**
		 * @brief Check if a file was modified since last check
		 * @return true if the file was modified, false otherwise
		 */
		bool WasConfigUpdated();

		/**
		 * @brief Get the instance of the TinyUSBMSC (singleton)
		 */
		static TinyUSBMSC *GetInstance() { return myInstance; }

		std::function<void()> myConfigUpdateCallback;
		std::function<void()> myEjectedCallback;

		// TinyUSB callbacks
		static bool MSCReadCallback(uint32_t lba, void *buffer, uint32_t offset, uint32_t bufsize);
		static bool MSCWriteCallback(uint32_t lba, uint8_t *buffer, uint32_t offset, uint32_t bufsize);
		static void MSCFlushCallback(void);

	private:
		std::shared_ptr<LittleFSSettings> myLittleFsSettings;

		bool myConfigUpdated;
		static TinyUSBMSC *myInstance;

		// Internal helper methods
		void UpdateConfigFromFS();
		void CheckForConfigUpdate();
	};
}