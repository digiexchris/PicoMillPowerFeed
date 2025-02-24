#pragma once

#include <cstdint>
#include <string>

namespace PowerFeed
{

	template <typename Derived>
	class IStepper
	{
	public:
		TODO : make this a base class that does most, and only the picostepper does the hardware things like en / dir gpio and the PIOStepper management things.use concepts to enforce the required functions.virtual void SetDirection(bool direction);
		virtual bool GetDirection();
		virtual bool GetTargetDirection();
		virtual uint32_t GetTargetSpeed();
		virtual void SetSpeed(uint32_t speed);
		virtual void Init();
		virtual uint32_t GetCurrentSpeed();
		virtual void Enable();
		virtual void Disable();
		virtual bool IsEnabled();
		virtual ~IStepper() = default;
		IStepper() = default;
	};

}
