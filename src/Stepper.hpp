#pragma once

#include <cstdint>
#include <string>

namespace PicoMill
{

	class IStepper
	{
	public:
		virtual void SetDirection(bool direction) = 0;
		virtual bool GetDirection() = 0;
		virtual bool GetTargetDirection() = 0;
		virtual uint32_t GetTargetSpeed() = 0;
		virtual void SetSpeed(uint32_t speed) = 0;
		virtual uint32_t GetSetSpeed() = 0;
		virtual void Init() = 0;
		virtual uint32_t GetCurrentSpeed() = 0;
		virtual void Enable() = 0;
		virtual void Disable() = 0;
		virtual bool IsEnabled() = 0;
		virtual void Update() = 0;
	};

}
