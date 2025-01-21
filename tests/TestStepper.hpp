#pragma once

#include "../src/Stepper.hpp"

#include <gmock/gmock.h>

class TestStepper : public IStepper
{
public:
	TestStepper(){};

	MOCK_METHOD(void, Init, (), (override));
	MOCK_METHOD(uint32_t, GetCurrentSpeed, (), (override));
	MOCK_METHOD(void, SetDirection, (bool direction), (override));
	MOCK_METHOD(void, Update, (), (override));
	MOCK_METHOD(bool, GetDirection, (), (override));
	MOCK_METHOD(bool, GetTargetDirection, (), (override));
	MOCK_METHOD(uint32_t, GetTargetSpeed, (), (override));
	MOCK_METHOD(void, SetSpeed, (uint32_t speed), (override));
	MOCK_METHOD(uint32_t, GetSetSpeed, (), (override));
	MOCK_METHOD(void, Enable, (), (override));
	MOCK_METHOD(void, Disable, (), (override));
	MOCK_METHOD(bool, IsEnabled, (), (override));
};
