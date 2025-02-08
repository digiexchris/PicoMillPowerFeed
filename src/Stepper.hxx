#pragma once

#include <stdint.h>
#include <string.h>

namespace PowerFeed
{

	class IStepper
	{
	public:
		virtual void SetAcceleration(uint32_t acceleration) = 0;
		virtual void SetDirection(bool direction) = 0;
		virtual bool GetDirection() = 0;
		virtual bool GetTargetDirection() = 0;
		virtual uint32_t GetTargetSpeed() = 0;
		virtual void SetSpeed(uint32_t speed) = 0;
		virtual uint32_t GetSetSpeed() = 0;
		virtual void Start() = 0;
		virtual uint32_t GetCurrentSpeed() = 0;
		virtual void Enable() = 0;
		virtual void Disable() = 0;
		virtual bool IsEnabled() = 0;
		virtual ~IStepper() = default;
		IStepper() = default;
	};

}
