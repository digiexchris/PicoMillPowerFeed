#include "Settings.hxx"
#include <memory>
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <stdexcept>

namespace PowerFeed
{
	void Settings::Driver::to_json(rapidjson::Writer<rapidjson::StringBuffer> &writer) const
	{
		writer.StartObject();
		writer.String("DRIVER_DIRECTION_CHANGE_DELAY_MS");
		writer.Uint(driverDirectionChangeDelayMs);
		writer.String("DRIVER_DIR_PIN");
		writer.Uint(driverDirPin);
		writer.String("DRIVER_EN_PIN");
		writer.Uint(driverEnPin);
		writer.String("DRIVER_STEP_PIN");
		writer.Uint(driverStepPin);
		writer.String("DRIVER_ENABLE_VALUE");
		writer.Bool(driverEnableValue);
		writer.String("DRIVER_DISABLE_TIMEOUT");
		writer.Uint(driverDisableTimeout);
		writer.EndObject();
	}

	Settings::Driver Settings::Driver::from_json(const rapidjson::Value &obj)
	{
		Driver s;
		s.driverDirectionChangeDelayMs = obj["DRIVER_DIRECTION_CHANGE_DELAY_MS"].GetUint();
		s.driverDirPin = obj["DRIVER_DIR_PIN"].GetUint();
		s.driverEnPin = obj["DRIVER_EN_PIN"].GetUint();
		s.driverStepPin = obj["DRIVER_STEP_PIN"].GetUint();
		s.driverEnableValue = obj["DRIVER_ENABLE_VALUE"].GetBool();
		s.driverDisableValue = !s.driverEnableValue;
		s.driverDisableTimeout = obj["DRIVER_DISABLE_TIMEOUT"].GetUint();
		return s;
	}

	void Settings::Display::to_json(rapidjson::Writer<rapidjson::StringBuffer> &writer) const
	{
		writer.StartObject();
		writer.String("USE_SSD1306");
		writer.Bool(useSsd1306);
		writer.String("SSD1306_ADDRESS");
		writer.Uint(ssd1306Address);
		writer.String("I2C_MASTER_SDA_IO");
		writer.Uint(i2cMasterSdaIo);
		writer.String("I2C_MASTER_SCL_IO");
		writer.Uint(i2cMasterSclIo);
		writer.String("I2C_MASTER_NUM");
		writer.Uint(i2cMasterNum);
		writer.EndObject();
	}

	Settings::Display Settings::Display::from_json(const rapidjson::Value &obj)
	{
		Display s;
		s.useSsd1306 = obj["USE_SSD1306"].GetBool();
		s.ssd1306Address = obj["SSD1306_ADDRESS"].GetUint();
		s.i2cMasterSdaIo = obj["I2C_MASTER_SDA_IO"].GetUint();
		s.i2cMasterSclIo = obj["I2C_MASTER_SCL_IO"].GetUint();
		s.i2cMasterNum = obj["I2C_MASTER_NUM"].GetUint();
		return s;
	}

	void Settings::Controls::to_json(rapidjson::Writer<rapidjson::StringBuffer> &writer) const
	{
		writer.StartObject();
		writer.String("LEFTPIN");
		writer.Uint(leftPin);
		writer.String("RIGHTPIN");
		writer.Uint(rightPin);
		writer.String("RAPIDPIN");
		writer.Uint(rapidPin);
		writer.String("ENCODER_A_PIN");
		writer.Uint(encoderAPin);
		writer.String("ENCODER_B_PIN");
		writer.Uint(encoderBPin);
		writer.String("ENCODER_BUTTON_PIN");
		writer.Uint(encoderButtonPin);
		writer.String("UNITS_SWITCH_DELAY_MS");
		writer.Uint(unitsSwitchDelayMs);
		writer.String("DEBOUNCE_DELAY_US");
		writer.Uint(debounceDelayUs);
		writer.String("ENCODER_COUNTS_TO_STEPS_PER_SECOND");
		writer.Uint(encoderCountsToStepsPerSecond);
		writer.String("ENCODER_INVERT");
		writer.Bool(encoderInvert);
		writer.EndObject();
	}

	Settings::Controls Settings::Controls::from_json(const rapidjson::Value &obj)
	{
		Controls s;
		s.leftPin = obj["LEFTPIN"].GetUint();
		s.rightPin = obj["RIGHTPIN"].GetUint();
		s.rapidPin = obj["RAPIDPIN"].GetUint();
		s.encoderAPin = obj["ENCODER_A_PIN"].GetUint();
		s.encoderBPin = obj["ENCODER_B_PIN"].GetUint();
		s.encoderButtonPin = obj["ENCODER_BUTTON_PIN"].GetUint();
		s.unitsSwitchDelayMs = obj["UNITS_SWITCH_DELAY_MS"].GetUint();
		s.debounceDelayUs = obj["DEBOUNCE_DELAY_US"].GetUint();
		s.encoderCountsToStepsPerSecond = obj["ENCODER_COUNTS_TO_STEPS_PER_SECOND"].GetUint();
		s.encoderInvert = obj["ENCODER_INVERT"].GetBool();
		return s;
	}

	void Settings::Mechanical::to_json(rapidjson::Writer<rapidjson::StringBuffer> &writer) const
	{
		writer.StartObject();
		writer.String("MAX_LEADSCREW_RPM");
		writer.Uint(maxLeadscrewRpm);
		writer.String("MAX_DRIVER_STEPS_PER_SECOND");
		writer.Uint(maxDriverStepsPerSecond);
		writer.String("STEPS_PER_MOTOR_REV");
		writer.Uint(stepsPerMotorRev);
		writer.String("MOTOR_TO_LEADSCREW_REDUCTION");
		writer.Double(motorToLeadscrewReduction);
		writer.String("ACCELERATION");
		writer.Uint(acceleration);
		writer.String("DECELERATION_MULTIPLIER");
		writer.Uint(decelerationMultiplier);
		writer.String("ACCELERATION_JERK");
		writer.Uint(accelerationJerk);
		writer.String("MOVE_LEFT_DIRECTION");
		writer.Bool(moveLeftDirection);
		writer.String("MM_PER_LEADSCREW_REV");
		writer.Double(mmPerLeadscrewRev);
		writer.EndObject();
	}

	Settings::Mechanical Settings::Mechanical::from_json(const rapidjson::Value &obj)
	{
		Mechanical s;
		s.maxLeadscrewRpm = obj["MAX_LEADSCREW_RPM"].GetUint();
		s.maxDriverStepsPerSecond = obj["MAX_DRIVER_STEPS_PER_SECOND"].GetUint();
		s.stepsPerMotorRev = obj["STEPS_PER_MOTOR_REV"].GetUint();
		s.motorToLeadscrewReduction = obj["MOTOR_TO_LEADSCREW_REDUCTION"].GetDouble();
		s.acceleration = obj["ACCELERATION"].GetUint();
		s.decelerationMultiplier = obj["DECELERATION_MULTIPLIER"].GetUint();
		s.accelerationJerk = obj["ACCELERATION_JERK"].GetUint();
		s.moveLeftDirection = obj["MOVE_LEFT_DIRECTION"].GetBool();
		s.mmPerLeadscrewRev = obj["MM_PER_LEADSCREW_REV"].GetDouble();
		s.moveRightDirection = !s.moveLeftDirection;
		s.maxStepsPerSecond = s.maxLeadscrewRpm * s.stepsPerMotorRev / 60;
		if (s.maxStepsPerSecond > s.maxDriverStepsPerSecond)
		{
			s.maxStepsPerSecond = s.maxDriverStepsPerSecond;
		}
		s.stepsPerMm = (s.stepsPerMotorRev * 4.055555556) / s.mmPerLeadscrewRev;
		s.moveRightDirection = !s.moveLeftDirection;

		return s;
	}

	void Settings::SavedSettings::to_json(rapidjson::Writer<rapidjson::StringBuffer> &writer) const
	{
		writer.StartObject();
		writer.String("NORMAL_SPEED");
		writer.Uint(normalSpeed);
		writer.String("RAPID_SPEED");
		writer.Uint(rapidSpeed);
		writer.String("INCH_UNITS");
		writer.Bool(inchUnits);
		writer.EndObject();
	}

	Settings::SavedSettings Settings::SavedSettings::from_json(const rapidjson::Value &obj)
	{
		SavedSettings s;
		s.normalSpeed = obj["NORMAL_SPEED"].GetUint();
		s.rapidSpeed = obj["RAPID_SPEED"].GetUint();
		s.inchUnits = obj["INCH_UNITS"].GetBool();
		return s;
	}

	void Settings::to_json(rapidjson::Writer<rapidjson::StringBuffer> &writer) const
	{
		writer.StartObject();
		writer.String("DRIVER");
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
		writer.String("MECHANICAL");
		writer.StartObject();
		mechanical.to_json(writer);
		writer.EndObject();
		writer.String("SAVED_SETTINGS");
		writer.StartObject();
		savedSettings.to_json(writer);
		writer.EndObject();
		writer.EndObject();
	}

	Settings Settings::from_json(const rapidjson::Document &doc)
	{
		Settings s;
		s.driver = Driver::from_json(doc["DRIVER"]);
		s.display = Display::from_json(doc["DISPLAY"]);
		s.controls = Controls::from_json(doc["CONTROLS"]);
		s.mechanical = Mechanical::from_json(doc["MECHANICAL"]);
		s.savedSettings = SavedSettings::from_json(doc["SAVED_SETTINGS"]);
		return s;
	}

	SettingsManager::SettingsManager()
	{
		Load();
	}

	std::shared_ptr<Settings> SettingsManager::GetDefaultSettings()
	{
		return myDefaultSettings;
	}

	std::shared_ptr<Settings> SettingsManager::Load()
	{
		std::string defaultSettingsJson = PicoMill::CONFIG_JSON;
		rapidjson::Document doc;

		try
		{

			doc.Parse(defaultSettingsJson.c_str());

			if (doc.HasParseError())
			{
				throw std::runtime_error("JSON parse error: " + std::to_string(doc.GetParseError()) +
										 " at offset " + std::to_string(doc.GetErrorOffset()));
			}

			myDefaultSettings = std::make_shared<Settings>(Settings::from_json(doc));
		}
		catch (const std::exception &e)
		{
			throw e;
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
