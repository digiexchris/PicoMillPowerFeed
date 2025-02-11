#pragma once

#include "Display.hxx"
#include "config.h"
#include <boost/json.hpp>
#include <cstdint>
#include <memory>

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

			boost::json::object to_json() const
			{
				boost::json::object obj;
				obj["DRIVER_DIRECTION_CHANGE_DELAY_MS"] = driver_direction_change_delay_ms;
				obj["DRIVER_DIR_PIN"] = driver_dir_pin;
				obj["DRIVER_EN_PIN"] = driver_en_pin;
				obj["DRIVER_STEP_PIN"] = driver_step_pin;
				obj["DRIVER_ENABLE_VALUE"] = driver_enable_value;
				obj["DRIVER_DISABLE_TIMEOUT"] = driver_disable_timeout;
				return obj;
			}

			static DriverSettings from_json(const boost::json::object &obj)
			{
				DriverSettings s;
				s.driver_direction_change_delay_ms = obj["DRIVER_DIRECTION_CHANGE_DELAY_MS"].as_int64();
				s.driver_dir_pin = obj["DRIVER_DIR_PIN"].as_int64();
				s.driver_en_pin = obj["DRIVER_EN_PIN"].as_int64();
				s.driver_step_pin = obj["DRIVER_STEP_PIN"].as_int64();
				s.driver_enable_value = obj["DRIVER_ENABLE_VALUE"].as_bool();
				s.driver_disable_timeout = obj["DRIVER_DISABLE_TIMEOUT"].as_int64();
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

			boost::json::object to_json() const
			{
				boost::json::object obj;
				obj["USE_SSD1306"] = use_ssd1306;
				obj["SSD1306_ADDRESS"] = ssd1306_address;
				obj["I2C_MASTER_SDA_IO"] = i2c_master_sda_io;
				obj["I2C_MASTER_SCL_IO"] = i2c_master_scl_io;
				obj["I2C_MASTER_NUM"] = i2c_master_num;
				return obj;
			}

			static DisplaySettings from_json(const boost::json::object &obj)
			{
				DisplaySettings s;
				s.use_ssd1306 = obj["USE_SSD1306"].as_bool();
				s.ssd1306_address = obj["SSD1306_ADDRESS"].as_int64();
				s.i2c_master_sda_io = obj["I2C_MASTER_SDA_IO"].as_int64();
				s.i2c_master_scl_io = obj["I2C_MASTER_SCL_IO"].as_int64();
				s.i2c_master_num = obj["I2C_MASTER_NUM"].as_int64();
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

			boost::json::object to_json() const
			{
				boost::json::object obj;
				obj["LEFTPIN"] = left_pin;
				obj["RIGHTPIN"] = right_pin;
				obj["RAPIDPIN"] = rapid_pin;
				obj["ENCODER_A_PIN"] = encoder_a_pin;
				obj["ENCODER_B_PIN"] = encoder_b_pin;
				obj["ENCODER_BUTTON_PIN"] = encoder_button_pin;
				obj["UNITS_SWITCH_DELAY_MS"] = units_switch_delay_ms;
				obj["DEBOUNCE_DELAY_US"] = debounce_delay_us;
				obj["ENCODER_COUNTS_TO_STEPS_PER_SECOND"] = encoder_counts_to_steps_per_second;
				obj["ENCODER_INVERT"] = encoder_invert;
				return obj;
			}

			static Controls from_json(const boost::json::object &obj)
			{
				Controls s;
				s.left_pin = obj["LEFTPIN"].as_int64();
				s.right_pin = obj["RIGHTPIN"].as_int64();
				s.rapid_pin = obj["RAPIDPIN"].as_int64();
				s.encoder_a_pin = obj["ENCODER_A_PIN"].as_int64();
				s.encoder_b_pin = obj["ENCODER_B_PIN"].as_int64();
				s.encoder_button_pin = obj["ENCODER_BUTTON_PIN"].as_int64();
				s.units_switch_delay_ms = obj["UNITS_SWITCH_DELAY_MS"].as_int64();
				s.debounce_delay_us = obj["DEBOUNCE_DELAY_US"].as_int64();
				s.encoder_counts_to_steps_per_second = obj["ENCODER_COUNTS_TO_STEPS_PER_SECOND"].as_int64();
				s.encoder_invert = obj["ENCODER_INVERT"].as_bool();
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

			boost::json::object to_json() const
			{
				boost::json::object obj;
				obj["MAX_LEADSCREW_RPM"] = max_leadscrew_rpm;
				obj["MAX_DRIVER_STEPS_PER_SECOND"] = max_driver_steps_per_second;
				obj["STEPS_PER_MOTOR_REV"] = steps_per_motor_rev;
				obj["STEPS_PER_LEADSCREW_REV"] = steps_per_leadscrew_rev;
				obj["ACCELERATION"] = acceleration;
				obj["DECELERATION_MULTIPLIER"] = deceleration_multiplier;
				obj["ACCELERATION_JERK"] = acceleration_jerk;
				obj["MOVE_LEFT_DIRECTION"] = move_left_direction;
				return obj;
			}

			static MechanicalParameters from_json(const boost::json::object &obj)
			{
				MechanicalParameters s;
				s.max_leadscrew_rpm = obj["MAX_LEADSCREW_RPM"].as_int64();
				s.max_driver_steps_per_second = obj["MAX_DRIVER_STEPS_PER_SECOND"].as_int64();
				s.steps_per_motor_rev = obj["STEPS_PER_MOTOR_REV"].as_int64();
				s.steps_per_leadscrew_rev = obj["STEPS_PER_LEADSCREW_REV"].as_double();
				s.acceleration = obj["ACCELERATION"].as_int64();
				s.deceleration_multiplier = obj["DECELERATION_MULTIPLIER"].as_int64();
				s.acceleration_jerk = obj["ACCELERATION_JERK"].as_int64();
				s.move_left_direction = obj["MOVE_LEFT_DIRECTION"].as_bool();
				return s;
			}
		};

		struct SavedSettings
		{
			uint32_t normal_speed;
			uint32_t rapid_speed;
			bool inch_units;

			boost::json::object to_json() const
			{
				boost::json::object obj;
				obj["NORMAL_SPEED"] = normal_speed;
				obj["RAPID_SPEED"] = rapid_speed;
				obj["INCH_UNITS"] = inch_units;
				return obj;
			}

			static SavedSettings from_json(const boost::json::object &obj)
			{
				SavedSettings s;
				s.normal_speed = obj["NORMAL_SPEED"].as_int64();
				s.rapid_speed = obj["RAPID_SPEED"].as_int64();
				s.inch_units = obj["INCH_UNITS"].as_bool();
				return s;
			}
		};

		DriverSettings driver;
		DisplaySettings display;
		Controls controls;
		MechanicalParameters mechanical_parameters;
		SavedSettings saved_settings;

		boost::json::object to_json() const
		{
			boost::json::object obj;
			obj["DRIVER_SETTINGS"] = driver.to_json();
			obj["DISPLAY"] = display.to_json();
			obj["CONTROLS"] = controls.to_json();
			obj["MECHANICAL_PARAMETERS"] = mechanical_parameters.to_json();
			obj["SAVED_SETTINGS"] = saved_settings.to_json();
			return obj;
		}

		static Settings from_json(const boost::json::value &json)
		{
			Settings s;
			auto obj = json.as_object();
			s.driver = DriverSettings::from_json(obj["DRIVER_SETTINGS"].as_object());
			s.display = DisplaySettings::from_json(obj["DISPLAY"].as_object());
			s.controls = Controls::from_json(obj["CONTROLS"].as_object());
			s.mechanical_parameters = MechanicalParameters::from_json(obj["MECHANICAL_PARAMETERS"].as_object());
			s.saved_settings = SavedSettings::from_json(obj["SAVED_SETTINGS"].as_object());
			return s;
		}
	};

	class SettingsManager
	{
	public:
		SettingsManager()
		{
			std::string defaultSettingsJson = PicoMill::CONFIG_JSON;

			try
			{
				boost::json::value parsed parsed = boost::json::parse(defaultSettingsJson);
				myDefaultSettings = std::make_shared<Settings>(Settings::from_json(parsed));
			}
			catch (const boost::json::system_error &e)
			{
				// Handle JSON parsing errors
				panic("JSON parse error: %s", e.what());
			}
			catch (const std::exception &e)
			{
				// Handle other errors
				panic("Error loading settings: %s", e.what());
			}
		}
		virtual std::shared_ptr<Settings> Load() = 0;
		virtual void Save(std::shared_ptr<Settings> settings) = 0;

	protected:
		std::shared_ptr<Settings> myDefaultSettings;
	};

} // namespace