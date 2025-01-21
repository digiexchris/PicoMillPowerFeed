#pragma once

#include <cstdint>
#include <string>

class IStepper
{
public:
	virtual void SetDirection(bool direction) = 0;
	virtual void Init() = 0;
	virtual uint32_t GetCurrentSpeed() = 0;
	virtual void Update() = 0;
};
