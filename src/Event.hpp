#pragma once
#include <cstdint>

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

struct Start : Event
{
	Start(bool aDirection) : direction(aDirection) { type = Type::START; }
	bool direction;
};

struct Stop : Event
{
	Stop() { type = Type::STOP; }
};

struct EnableRapid : Event
{
	EnableRapid() { type = Type::ENABLE_RAPID; }
};

struct DisableRapid : Event
{
	DisableRapid() { type = Type::DISABLE_RAPID; }
};

struct SetRapidSpeed : Event
{
	SetRapidSpeed(uint32_t aSpeed) : speed(aSpeed) { type = Type::SET_RAPID_SPEED; }
	uint32_t speed;
};

struct SetNormalSpeed : Event
{
	SetNormalSpeed(uint32_t aSpeed) : speed(aSpeed) { type = Type::SET_NORMAL_SPEED; }
	uint32_t speed;
};