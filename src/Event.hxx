#pragma once
#include <cstdint>

namespace PowerFeed
{
	struct Event
	{
		enum class Type
		{
			START,
			STOP,
			ENABLE_RAPID,
			DISABLE_RAPID,
			SET_RAPID_SPEED,
			SET_NORMAL_SPEED

		};

		Type type;
	};

	struct StartEvent : Event
	{
		StartEvent(bool aDirection) : direction(aDirection) { type = Type::START; }
		bool direction;
	};

	struct StopEvent : Event
	{
		StopEvent() { type = Type::STOP; }
	};

	struct EnableRapidEvent : Event
	{
		EnableRapidEvent() { type = Type::ENABLE_RAPID; }
	};

	struct DisableRapidEvent : Event
	{
		DisableRapidEvent() { type = Type::DISABLE_RAPID; }
	};

	struct SetRapidSpeedEvent : Event
	{
		SetRapidSpeedEvent(uint32_t aSpeed) : speed(aSpeed) { type = Type::SET_RAPID_SPEED; }
		uint32_t speed;
	};

	struct SetNormalSpeedEvent : Event
	{
		SetNormalSpeedEvent(uint32_t aSpeed) : speed(aSpeed) { type = Type::SET_NORMAL_SPEED; }
		uint32_t speed;
	};

} // namespace