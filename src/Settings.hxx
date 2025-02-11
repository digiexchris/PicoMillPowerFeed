#pragma once

#include "Display.hxx"
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
		struct DriverSettings
		{
			uint32_t driver_direction_change_delay_ms;
			uint16_t driver_dir_pin;
			uint16_t driver_en_pin;
			uint16_t driver_step_pin;
			bool driver_enable_value;
			uint16_t driver_disable_timeout;

			void to_json(rapidjson::Writer<rapidjson::StringBuffer> &writer) const
			{
				writer.StartObject();
				writer.String("DRIVER_DIRECTION_CHANGE_DELAY_MS");
				writer.Uint(driver_direction_change_delay_ms);
				writer.String("DRIVER_DIR_PIN");
				writer.Uint(driver_dir_pin);
				writer.String("DRIVER_EN_PIN");
				writer.Uint(driver_en_pin);
				writer.String("DRIVER_STEP_PIN");
				writer.Uint(driver_step_pin);
				writer.String("DRIVER_ENABLE_VALUE");
				writer.Bool(driver_enable_value);
				writer.String("DRIVER_DISABLE_TIMEOUT");
				writer.Uint(driver_disable_timeout);
				writer.EndObject();
			}

			static DriverSettings from_json(const rapidjson::Value &obj)
			{
				DriverSettings s;
				s.driver_direction_change_delay_ms = obj["DRIVER_DIRECTION_CHANGE_DELAY_MS"].GetUint();
				s.driver_dir_pin = obj["DRIVER_DIR_PIN"].GetUint();
				s.driver_en_pin = obj["DRIVER_EN_PIN"].GetUint();
				s.driver_step_pin = obj["DRIVER_STEP_PIN"].GetUint();
				s.driver_enable_value = obj["DRIVER_ENABLE_VALUE"].GetBool();
				s.driver_disable_timeout = obj["DRIVER_DISABLE_TIMEOUT"].GetUint();
				return s;
			}
		};

		struct DisplaySettings
		{
			bool use_ssd1306;
			uint8_t ssd1306_address;
			uint8_t i2c_master_sda_io;
			uint8_t i2c_master_scl_io;
			uint8_t i2c_master_num;

			void to_json(rapidjson::Writer<rapidjson::StringBuffer> &writer) const
			{
				writer.StartObject();
				writer.String("USE_SSD1306");
				writer.Bool(use_ssd1306);
				writer.String("SSD1306_ADDRESS");
				writer.Uint(ssd1306_address);
				writer.String("I2C_MASTER_SDA_IO");
				writer.Uint(i2c_master_sda_io);
				writer.String("I2C_MASTER_SCL_IO");
				writer.Uint(i2c_master_scl_io);
				writer.String("I2C_MASTER_NUM");
				writer.Uint(i2c_master_num);
				writer.EndObject();
			}

			static DisplaySettings from_json(const rapidjson::Value &obj)
			{
				DisplaySettings s;
				s.use_ssd1306 = obj["USE_SSD1306"].GetBool();
				s.ssd1306_address = obj["SSD1306_ADDRESS"].GetUint();
				s.i2c_master_sda_io = obj["I2C_MASTER_SDA_IO"].GetUint();
				s.i2c_master_scl_io = obj["I2C_MASTER_SCL_IO"].GetUint();
				s.i2c_master_num = obj["I2C_MASTER_NUM"].GetUint();
				return s;
			}
		};

		struct Controls
		{
			uint16_t left_pin;
			uint16_t right_pin;
			uint16_t rapid_pin;
			uint16_t encoder_a_pin;
			uint16_t encoder_b_pin;
			uint16_t encoder_button_pin;
			uint32_t units_switch_delay_ms;
			uint32_t debounce_delay_us;
			uint16_t encoder_counts_to_steps_per_second;
			bool encoder_invert;

			void to_json(rapidjson::Writer<rapidjson::StringBuffer> &writer) const
			{
				writer.StartObject();
				writer.String("LEFTPIN");
				writer.Uint(left_pin);
				writer.String("RIGHTPIN");
				writer.Uint(right_pin);
				writer.String("RAPIDPIN");
				writer.Uint(rapid_pin);
				writer.String("ENCODER_A_PIN");
				writer.Uint(encoder_a_pin);
				writer.String("ENCODER_B_PIN");
				writer.Uint(encoder_b_pin);
				writer.String("ENCODER_BUTTON_PIN");
				writer.Uint(encoder_button_pin);
				writer.String("UNITS_SWITCH_DELAY_MS");
				writer.Uint(units_switch_delay_ms);
				writer.String("DEBOUNCE_DELAY_US");
				writer.Uint(debounce_delay_us);
				writer.String("ENCODER_COUNTS_TO_STEPS_PER_SECOND");
				writer.Uint(encoder_counts_to_steps_per_second);
				writer.String("ENCODER_INVERT");
				writer.Bool(encoder_invert);
				writer.EndObject();
			}

			static Controls from_json(const rapidjson::Value &obj)
			{
				Controls s;
				s.left_pin = obj["LEFTPIN"].GetUint();
				s.right_pin = obj["RIGHTPIN"].GetUint();
				s.rapid_pin = obj["RAPIDPIN"].GetUint();
				s.encoder_a_pin = obj["ENCODER_A_PIN"].GetUint();
				s.encoder_b_pin = obj["ENCODER_B_PIN"].GetUint();
				s.encoder_button_pin = obj["ENCODER_BUTTON_PIN"].GetUint();
				s.units_switch_delay_ms = obj["UNITS_SWITCH_DELAY_MS"].GetUint();
				s.debounce_delay_us = obj["DEBOUNCE_DELAY_US"].GetUint();
				s.encoder_counts_to_steps_per_second = obj["ENCODER_COUNTS_TO_STEPS_PER_SECOND"].GetUint();
				s.encoder_invert = obj["ENCODER_INVERT"].GetBool();
				return s;
			}
		};

		struct MechanicalParameters
		{
			uint32_t max_leadscrew_rpm;
			uint32_t max_driver_steps_per_second;
			uint32_t steps_per_motor_rev;
			double steps_per_leadscrew_rev;
			uint32_t acceleration;
			uint8_t deceleration_multiplier;
			uint8_t acceleration_jerk;
			bool move_left_direction;

			void to_json(rapidjson::Writer<rapidjson::StringBuffer> &writer) const
			{
				writer.StartObject();
				writer.String("MAX_LEADSCREW_RPM");
				writer.Uint(max_leadscrew_rpm);
				writer.String("MAX_DRIVER_STEPS_PER_SECOND");
				writer.Uint(max_driver_steps_per_second);
				writer.String("STEPS_PER_MOTOR_REV");
				writer.Uint(steps_per_motor_rev);
				writer.String("STEPS_PER_LEADSCREW_REV");
				writer.Double(steps_per_leadscrew_rev);
				writer.String("ACCELERATION");
				writer.Uint(acceleration);
				writer.String("DECELERATION_MULTIPLIER");
				writer.Uint(deceleration_multiplier);
				writer.String("ACCELERATION_JERK");
				writer.Uint(acceleration_jerk);
				writer.String("MOVE_LEFT_DIRECTION");
				writer.Bool(move_left_direction);
				writer.EndObject();
			}

			static MechanicalParameters from_json(const rapidjson::Value &obj)
			{
				MechanicalParameters s;
				s.max_leadscrew_rpm = obj["MAX_LEADSCREW_RPM"].GetUint();
				s.max_driver_steps_per_second = obj["MAX_DRIVER_STEPS_PER_SECOND"].GetUint();
				s.steps_per_motor_rev = obj["STEPS_PER_MOTOR_REV"].GetUint();
				s.steps_per_leadscrew_rev = obj["STEPS_PER_LEADSCREW_REV"].GetDouble();
				s.acceleration = obj["ACCELERATION"].GetUint();
				s.deceleration_multiplier = obj["DECELERATION_MULTIPLIER"].GetUint();
				s.acceleration_jerk = obj["ACCELERATION_JERK"].GetUint();
				s.move_left_direction = obj["MOVE_LEFT_DIRECTION"].GetBool();
				return s;
			}
		};

		struct SavedSettings
		{
			uint32_t normal_speed;
			uint32_t rapid_speed;
			bool inch_units;

			void to_json(rapidjson::Writer<rapidjson::StringBuffer> &writer) const
			{
				writer.StartObject();
				writer.String("NORMAL_SPEED");
				writer.Uint(normal_speed);
				writer.String("RAPID_SPEED");
				writer.Uint(rapid_speed);
				writer.String("INCH_UNITS");
				writer.Bool(inch_units);
				writer.EndObject();
			}

			static SavedSettings from_json(const rapidjson::Value &obj)
			{
				SavedSettings s;
				s.normal_speed = obj["NORMAL_SPEED"].GetUint();
				s.rapid_speed = obj["RAPID_SPEED"].GetUint();
				s.inch_units = obj["INCH_UNITS"].GetBool();
				return s;
			}
		};

		DriverSettings driver;
		DisplaySettings display;
		Controls controls;
		MechanicalParameters mechanical_parameters;
		SavedSettings saved_settings;

		void to_json(rapidjson::Writer<rapidjson::StringBuffer> &writer) const
		{
			writer.StartObject();
			writer.String("DRIVER_SETTINGS");
			writer.StartObject();
			driver.to_json(writer);
			writer.EndObject();
			writer.String("DISPLAY");
			writer.StartObject();
			display.to_json(writer);
			writer.EndObject();
			writer.String("CONTROLS");
			writer.StartObject();
			controls.to_json(writer);
			writer.EndObject();
			writer.String("MECHANICAL_PARAMETERS");
			writer.StartObject();
			mechanical_parameters.to_json(writer);
			writer.EndObject();
			writer.String("SAVED_SETTINGS");
			writer.StartObject();
			saved_settings.to_json(writer);
			writer.EndObject();
			writer.EndObject();
		}

		static Settings from_json(const rapidjson::Document &doc)
		{
			Settings s;
			s.driver = DriverSettings::from_json(doc["DRIVER_SETTINGS"]);
			s.display = DisplaySettings::from_json(doc["DISPLAY"]);
			s.controls = Controls::from_json(doc["CONTROLS"]);
			s.mechanical_parameters = MechanicalParameters::from_json(doc["MECHANICAL_PARAMETERS"]);
			s.saved_settings = SavedSettings::from_json(doc["SAVED_SETTINGS"]);
			return s;
		}
	};

	class SettingsManager
	{
	public:
		SettingsManager()
		{
			std::string defaultSettingsJson = PicoMill::CONFIG_JSON;
			rapidjson::Document doc;
			doc.Parse(defaultSettingsJson.c_str());

			if (doc.HasParseError())
			{
				panic("JSON parse error: %u at offset %zu",
					  doc.GetParseError(),
					  doc.GetErrorOffset());
			}

			try
			{
				myDefaultSettings = std::make_shared<Settings>(Settings::from_json(doc));
			}
			catch (const std::exception &e)
			{
				panic("Error loading settings: %s", e.what());
			}
		}

	private:
		std::shared_ptr<Settings> myDefaultSettings;
	};
}