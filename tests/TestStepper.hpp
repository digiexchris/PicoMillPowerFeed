#pragma once

#include "../src/Stepper.hpp"

class TestStepper : public IStepper
{
public:
	TestStepper(){};
	virtual void Init() override
	{
	}

	virtual uint32_t GetCurrentSpeed() override { return 0; };

	virtual void SetDirection(bool direction) override{};

	// call this as fast as possible
	virtual void Update() override{

	};
};