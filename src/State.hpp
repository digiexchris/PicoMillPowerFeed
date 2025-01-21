#pragma once

#include "Stepper.hpp"
#include <memory>
#include <stdint.h>

enum class States
{
	ACCELERATING,
	COASTING,
	DECELERATING,
	STOPPED
};

struct Command
{
	enum class Type
	{
		START,
		STOP,
		CHANGE_SPEED
	};

	Type type;
};

struct ChangeSpeed : Command
{
	ChangeSpeed(uint32_t speed) : speed(speed) { type = Type::CHANGE_SPEED; }
	uint32_t speed;
};

struct Stop : Command
{
	Stop() { type = Type::STOP; }
};

struct Start : Command
{
	Start(bool aDirection, uint32_t aSpeed) : direction(aDirection), speed(aSpeed) { type = Type::START; }
	bool direction;
	uint32_t speed;
};

class State
{
public:
	State(std::shared_ptr<IStepper> aStepper);

	void ProcessCommand(Command command);
	States GetState() { return myState; }

private:
	void ProcessStop();
	void ProcessNewSpeed(uint32_t speed);
	void ProcessStart(bool direction, uint32_t speed);
	void Run();

	States myState;
	uint32_t mySpeed;
	uint32_t myTargetSpeed;
	bool myDirection;
	bool myTargetDirection;

	std::shared_ptr<IStepper> myStepper;
};