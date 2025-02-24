#include "Settings.hxx"
#include <memory>
#include <stdexcept>

namespace PowerFeed
{
	nlohmann::json Settings::Driver::to_json() const
	{
		return {
			{"DRIVER_DIRECTION_CHANGE_DELAY_MS", driverDirectionChangeDelayMs},
			{"DRIVER_DIR_PIN", driverDirPin},
			{"DRIVER_EN_PIN", driverEnPin},
			{"DRIVER_STEP_PIN", driverStepPin},
			{"DRIVER_ENABLE_VALUE", driverEnableValue},
			{"DRIVER_DISABLE_TIMEOUT", driverDisableTimeout}};
	}

	Settings::Driver Settings::Driver::from_json(const nlohmann::json &j)
	{
		Driver s;
		s.driverDirectionChangeDelayMs = j["DRIVER_DIRECTION_CHANGE_DELAY_MS"].get<uint32_t>();
		s.driverDirPin = j["DRIVER_DIR_PIN"].get<uint16_t>();
		s.driverEnPin = j["DRIVER_EN_PIN"].get<uint16_t>();
		s.driverStepPin = j["DRIVER_STEP_PIN"].get<uint16_t>();
		s.driverEnableValue = j["DRIVER_ENABLE_VALUE"].get<bool>();
		s.driverDisableValue = !s.driverEnableValue;
		s.driverDisableTimeout = j["DRIVER_DISABLE_TIMEOUT"].get<uint16_t>();
		return s;
	}

	nlohmann::json Settings::Display::to_json() const
	{
		return {
			{"USE_SSD1306", useSsd1306},
			{"SSD1306_ADDRESS", ssd1306Address},
			{"SSD1306_ROTATE_180", ssd1306Rotate180},
			{"I2C_MASTER_SDA_IO", i2cMasterSdaIo},
			{"I2C_MASTER_SCL_IO", i2cMasterSclIo},
			{"I2C_MASTER_NUM", i2cMasterNum}};
	}

	Settings::Display Settings::Display::from_json(const nlohmann::json &j)
	{
		Display s;
		s.useSsd1306 = j["USE_SSD1306"].get<bool>();
		s.ssd1306Address = j["SSD1306_ADDRESS"].get<uint8_t>();
		s.ssd1306Rotate180 = j["SSD1306_ROTATE_180"].get<bool>();
		s.i2cMasterSdaIo = j["I2C_MASTER_SDA_IO"].get<uint8_t>();
		s.i2cMasterSclIo = j["I2C_MASTER_SCL_IO"].get<uint8_t>();
		s.i2cMasterNum = j["I2C_MASTER_NUM"].get<uint8_t>();
		return s;
	}

	nlohmann::json Settings::Controls::to_json() const
	{
		return {
			{"LEFTPIN", leftPin},
			{"RIGHTPIN", rightPin},
			{"RAPIDPIN", rapidPin},
			{"ENCODER_A_PIN", encoderAPin},
			{"ENCODER_B_PIN", encoderBPin},
			{"ENCODER_BUTTON_PIN", encoderButtonPin},
			{"UNITS_SWITCH_DELAY_MS", unitsSwitchDelayMs},
			{"DEBOUNCE_DELAY_US", debounceDelayUs},
			{"ENCODER_COUNTS_TO_STEPS_PER_SECOND", encoderCountsToStepsPerSecond},
			{"ENCODER_INVERT", encoderInvert}};
	}

	Settings::Controls Settings::Controls::from_json(const nlohmann::json &j)
	{
		Controls s;
		s.leftPin = j["LEFTPIN"].get<uint16_t>();
		s.rightPin = j["RIGHTPIN"].get<uint16_t>();
		s.rapidPin = j["RAPIDPIN"].get<uint16_t>();
		s.encoderAPin = j["ENCODER_A_PIN"].get<uint16_t>();
		s.encoderBPin = j["ENCODER_B_PIN"].get<uint16_t>();
		s.encoderButtonPin = j["ENCODER_BUTTON_PIN"].get<uint16_t>();
		s.unitsSwitchDelayMs = j["UNITS_SWITCH_DELAY_MS"].get<uint32_t>();
		s.debounceDelayUs = j["DEBOUNCE_DELAY_US"].get<uint32_t>();
		s.encoderCountsToStepsPerSecond = j["ENCODER_COUNTS_TO_STEPS_PER_SECOND"].get<uint16_t>();
		s.encoderInvert = j["ENCODER_INVERT"].get<bool>();
		return s;
	}

	nlohmann::json Settings::Mechanical::to_json() const
	{
		return {
			{"MAX_LEADSCREW_RPM", maxLeadscrewRpm},
			{"MAX_DRIVER_STEPS_PER_SECOND", maxDriverStepsPerSecond},
			{"STEPS_PER_MOTOR_REV", stepsPerMotorRev},
			{"MOTOR_TO_LEADSCREW_REDUCTION", motorToLeadscrewReduction},
			{"ACCELERATION", acceleration},
			{"DECELERATION", deceleration},
			{"ACCELERATION_JERK", accelerationJerk},
			{"MOVE_LEFT_DIRECTION", moveLeftDirection},
			{"MM_PER_LEADSCREW_REV", mmPerLeadscrewRev}};
	}

	Settings::Mechanical Settings::Mechanical::from_json(const nlohmann::json &j)
	{
		Mechanical s;
		s.maxLeadscrewRpm = j["MAX_LEADSCREW_RPM"].get<uint32_t>();
		s.maxDriverStepsPerSecond = j["MAX_DRIVER_STEPS_PER_SECOND"].get<uint32_t>();
		s.stepsPerMotorRev = j["STEPS_PER_MOTOR_REV"].get<uint32_t>();
		s.motorToLeadscrewReduction = j["MOTOR_TO_LEADSCREW_REDUCTION"].get<double>();
		s.acceleration = j["ACCELERATION"].get<uint32_t>();
		s.deceleration = j["DECELERATION"].get<uint32_t>();
		s.accelerationJerk = j["ACCELERATION_JERK"].get<uint8_t>();
		s.moveLeftDirection = j["MOVE_LEFT_DIRECTION"].get<bool>();
		s.mmPerLeadscrewRev = j["MM_PER_LEADSCREW_REV"].get<double>();

		s.moveRightDirection = !s.moveLeftDirection;
		s.maxStepsPerSecond = s.maxLeadscrewRpm * s.stepsPerMotorRev / 60;
		if (s.maxStepsPerSecond > s.maxDriverStepsPerSecond)
		{
			s.maxStepsPerSecond = s.maxDriverStepsPerSecond;
		}
		s.stepsPerMm = (s.stepsPerMotorRev * 4.055555556) / s.mmPerLeadscrewRev;
		return s;
	}

	nlohmann::json Settings::SavedSettings::to_json() const
	{
		return {
			{"NORMAL_SPEED", normalSpeed},
			{"RAPID_SPEED", rapidSpeed},
			{"INCH_UNITS", inchUnits}};
	}

	Settings::SavedSettings Settings::SavedSettings::from_json(const nlohmann::json &j)
	{
		SavedSettings s;
		s.normalSpeed = j["NORMAL_SPEED"].get<uint32_t>();
		s.rapidSpeed = j["RAPID_SPEED"].get<uint32_t>();
		s.inchUnits = j["INCH_UNITS"].get<bool>();
		return s;
	}

	nlohmann::json Settings::to_json() const
	{
		return {
			{"DRIVER", driver.to_json()},
			{"DISPLAY", display.to_json()},
			{"CONTROLS", controls.to_json()},
			{"MECHANICAL", mechanical.to_json()},
			{"SAVED_SETTINGS", savedSettings.to_json()}};
	}

	Settings Settings::from_json(const nlohmann::json &j)
	{
		Settings s;
		s.driver = Driver::from_json(j["DRIVER"]);
		s.display = Display::from_json(j["DISPLAY"]);
		s.controls = Controls::from_json(j["CONTROLS"]);
		s.mechanical = Mechanical::from_json(j["MECHANICAL"]);
		s.savedSettings = SavedSettings::from_json(j["SAVED_SETTINGS"]);
		return s;
	}

	SettingsManager::SettingsManager()
	{
	}

	std::shared_ptr<Settings> SettingsManager::GetDefaultSettings()
	{
		return myDefaultSettings;
	}

	std::shared_ptr<Settings> SettingsManager::Load()
	{
		std::string defaultSettingsJson = PicoMill::CONFIG_JSON;

		try
		{
			auto j = nlohmann::json::parse(defaultSettingsJson);
			myDefaultSettings = std::make_shared<Settings>(Settings::from_json(j));
		}
		catch (const nlohmann::json::parse_error &e)
		{
			throw std::runtime_error("JSON parse error: " + std::string(e.what()));
		}

		return myDefaultSettings;
	}

	void SettingsManager::Save(std::shared_ptr<Settings> settings)
	{
		// no-op for default settings
	}

	std::shared_ptr<Settings> SettingsManager::Get()
	{
		return myDefaultSettings;
	}
}
