#pragma once

#include "../src/Stepper.hpp"

#include <gmock/gmock.h>


class TestStepper : public IStepper
{
public:
	TestStepper(){};
	// virtual void Init() override
	// {
	// }

	// virtual uint32_t GetCurrentSpeed() override { return 0; };

	// virtual void SetDirection(bool direction) override{};

	// // call this as fast as possible
	// virtual void Update() override{

	// };

	MOCK_METHOD(void, Init, (), (override));
	MOCK_METHOD(uint32_t, GetCurrentSpeed, (), (override));
	MOCK_METHOD(void, SetDirection, (bool direction), (override));
	MOCK_METHOD(void, Update, (), (override));
};	
