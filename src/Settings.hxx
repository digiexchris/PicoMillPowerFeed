#pragma once

#include <cstdint>
#include <memory>
#include <nlohmann/json.hpp>

extern "C"
{
#include "FreeRTOS.h"
#include "timers.h"
}

namespace PowerFeed
{
	struct Settings
	{
		struct System
		{
			uint32_t settingsAutoSaveDelayMs;

			nlohmann::json to_json() const;
			static System from_json(const nlohmann::json &j);
		};

		struct Driver
		{
			uint32_t driverDirectionChangeDelayMs;
			uint16_t driverDirPin;
			uint16_t driverEnPin;
			uint16_t driverStepPin;
			bool driverEnableValue;
			bool driverDisableValue;
			uint16_t driverDisableTimeout;

			nlohmann::json to_json() const;
			static Driver from_json(const nlohmann::json &j);
		};

		struct Display
		{
			bool useSsd1306;
			uint8_t ssd1306Address;
			uint8_t i2cMasterSdaIo;
			uint8_t i2cMasterSclIo;
			uint8_t i2cMasterNum;
			bool ssd1306Rotate180;

			nlohmann::json to_json() const;
			static Display from_json(const nlohmann::json &j);
		};

		struct Controls
		{
			uint16_t leftPin;
			uint16_t rightPin;
			uint16_t rapidPin;
			uint16_t encoderAPin;
			uint16_t encoderBPin;
			uint16_t encoderButtonPin;
			uint32_t unitsSwitchDelayMs;
			uint32_t debounceDelayUs;
			uint16_t encoderCountsToStepsPerSecond;
			bool encoderInvert;

			nlohmann::json to_json() const;
			static Controls from_json(const nlohmann::json &j);
		};

		struct Mechanical
		{
			uint32_t maxLeadscrewRpm;
			uint32_t maxDriverStepsPerSecond;
			uint32_t stepsPerMotorRev;
			double stepsPerLeadscrewRev;
			uint32_t acceleration;
			uint32_t deceleration;
			uint8_t accelerationJerk;
			float motorToLeadscrewReduction;
			bool moveLeftDirection;

			// calculated after parse
			bool moveRightDirection;
			int32_t maxStepsPerSecond;
			float mmPerLeadscrewRev;
			float stepsPerMm;

			nlohmann::json to_json() const;
			static Mechanical from_json(const nlohmann::json &j);
		};

		struct SavedSettings
		{
			uint32_t normalSpeed;
			uint32_t rapidSpeed;
			bool inchUnits;

			nlohmann::json to_json() const;
			static SavedSettings from_json(const nlohmann::json &j);
		};

		System system;
		Driver driver;
		Display display;
		Controls controls;
		Mechanical mechanical;
		SavedSettings savedSettings;

		nlohmann::json to_json() const;
		static Settings from_json(const nlohmann::json &j);
	};

	class SettingsManager
	{
	public:
		SettingsManager();
		virtual ~SettingsManager();

		/**
		@brief Read from non volatile storage and return */
		virtual std::shared_ptr<Settings> Load();

		/**
		@brief Save settings to storage */
		virtual void Save(std::shared_ptr<Settings> settings);

		/**
		@brief Immediately save current settings without waiting for timer */
		virtual void SaveNow();

		/**
		@brief Get default settings */
		std::shared_ptr<Settings> GetDefaultSettings();

		/**
		@brief Return previously retrieved settings */
		virtual std::shared_ptr<Settings> Get();

		/**
		@brief Update a specific setting and schedule auto-save
		@param key The setting key to update (e.g. "NORMAL_SPEED", "RAPID_SPEED", "INCH_UNITS")
		@param value The new value for the setting
		@return true if the setting was updated, false otherwise
		*/
		template <typename T>
		bool Set(const std::string &key, T value);

		/**
		@brief Set multiple saved settings at once and schedule auto-save
		@param normalSpeed The normal speed value
		@param rapidSpeed The rapid speed value
		@param inchUnits The units setting (true = inch, false = mm)
		*/
		void SetSavedSettings(uint32_t normalSpeed, uint32_t rapidSpeed, bool inchUnits);

		/**
		@brief Schedule auto-save with current timer period
		*/
		void ScheduleAutoSave();

	protected:
		std::shared_ptr<Settings> myDefaultSettings;
		TimerHandle_t myAutoSaveTimer;

		/**
		@brief Timer callback for auto-save
		*/
		static void AutoSaveTimerCallback(TimerHandle_t xTimer);
	};
} // namespace PowerFeed
