#pragma once

#include "config.h"
#include <cstdint>
#include <memory>
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

namespace PowerFeed
{
	struct Settings
	{
		struct Driver
		{
			uint32_t driverDirectionChangeDelayMs;
			uint16_t driverDirPin;
			uint16_t driverEnPin;
			uint16_t driverStepPin;
			bool driverEnableValue;
			bool driverDisableValue;
			uint16_t driverDisableTimeout;

			void to_json(rapidjson::Writer<rapidjson::StringBuffer> &writer) const;
			static Driver from_json(const rapidjson::Value &obj);
		};

		struct Display
		{
			bool useSsd1306;
			uint8_t ssd1306Address;
			uint8_t i2cMasterSdaIo;
			uint8_t i2cMasterSclIo;
			uint8_t i2cMasterNum;
			bool ssd1306Rotate180;

			void to_json(rapidjson::Writer<rapidjson::StringBuffer> &writer) const;
			static Display from_json(const rapidjson::Value &obj);
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

			void to_json(rapidjson::Writer<rapidjson::StringBuffer> &writer) const;
			static Controls from_json(const rapidjson::Value &obj);
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

			void to_json(rapidjson::Writer<rapidjson::StringBuffer> &writer) const;
			static Mechanical from_json(const rapidjson::Value &obj);
		};

		struct SavedSettings
		{
			uint32_t normalSpeed;
			uint32_t rapidSpeed;
			bool inchUnits;

			void to_json(rapidjson::Writer<rapidjson::StringBuffer> &writer) const;
			static SavedSettings from_json(const rapidjson::Value &obj);
		};

		Driver driver;
		Display display;
		Controls controls;
		Mechanical mechanical;
		SavedSettings savedSettings;

		void to_json(rapidjson::Writer<rapidjson::StringBuffer> &writer) const;
		static Settings from_json(const rapidjson::Document &doc);
	};

	class SettingsManager
	{
	public:
		SettingsManager();
		/**
		@brief Read from non volatile storage and return */
		virtual std::shared_ptr<Settings> Load();
		virtual void Save(std::shared_ptr<Settings> settings);
		std::shared_ptr<Settings> GetDefaultSettings();
		/**
		@brief Return previously retrieved settings */
		virtual std::shared_ptr<Settings> Get();

	protected:
		std::shared_ptr<Settings> myDefaultSettings;
	};
} // namespace PowerFeed