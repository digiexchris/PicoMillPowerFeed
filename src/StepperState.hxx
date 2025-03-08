// #pragma once

// #include "Common.hxx"
// #include "Mutex.hxx"
// #include "Settings.hxx"
// #include "Stepper.hxx"
// #include <cstdlib>
// #include <memory>
// #include <sys/stat.h>

// #include <stdint.h>

// namespace PowerFeed
// {

// 	enum class States
// 	{
// 		STOPPED,
// 		STOPPING,
// 		ACCELERATING,
// 		COASTING,
// 		DECELERATING
// 	};

// 	struct Command
// 	{
// 		enum class Type
// 		{
// 			START,
// 			STOP,
// 			CHANGE_SPEED
// 		};

// 		Type type;
// 	};

// 	struct ChangeSpeed : Command
// 	{
// 		ChangeSpeed(uint32_t speed) : speed(speed) { type = Type::CHANGE_SPEED; }
// 		uint32_t speed;
// 	};

// 	struct Stop : Command
// 	{
// 		Stop() { type = Type::STOP; }
// 	};

// 	struct Start : Command
// 	{
// 		Start(bool aDirection, uint32_t aSpeed) : direction(aDirection), speed(aSpeed) { type = Type::START; }
// 		bool direction;
// 		uint32_t speed;
// 	};

// 	template <typename DerivedStepper>
// 	class StepperState
// 	{
// 	public:
// 		StepperState(SettingsManager *aSettings, StepperType<DerivedStepper> *aStepper)
// 		{
// 			mySettings = aSettings;
// 			myStepper = aStepper;
// 			myState = States::STOPPED;
// 			myTargetDirection = false;
// 			myRequestedSpeed = 0;
// 		}

// 		virtual void ProcessCommand(const Command &aCommand)
// 		{
// 			LockGuard<DefaultMutex> lock(myMutex);
// 			switch (aCommand.type)
// 			{
// 			case Command::Type::STOP:
// 				ProcessStop();
// 				break;
// 			case Command::Type::CHANGE_SPEED:
// 			{
// 				const ChangeSpeed &changeSpeed = static_cast<const ChangeSpeed &>(aCommand);
// 				ProcessNewSpeed(changeSpeed.speed);
// 			}
// 			break;

// 			case Command::Type::START:
// 			{
// 				const Start &start = static_cast<const Start &>(aCommand);
// 				ProcessStart(start.direction, start.speed);
// 			}
// 			break;
// 			}
// 		}

// 		virtual States GetState()
// 		{
// 			LockGuard<DefaultMutex> lock(myMutex);
// 			return myState;
// 		}

// 	private:
// 		void ProcessStop();
// 		void ProcessNewSpeed(uint32_t speed);
// 		void ProcessStart(bool direction, uint32_t speed);

// 		States myState;
// 		uint32_t myRequestedSpeed;
// 		bool myTargetDirection;
// 		StepperType<DerivedStepper> *myStepper;
// 		SettingsManager *mySettings;
// 		DefaultMutex myMutex; // Mutex to protect concurrent access
// 	};

// 	// Template implementations

// 	template <typename DerivedStepper>
// 	void StepperState<DerivedStepper>::ProcessStop()
// 	{
// 		// No need for mutex here as it's called from within ProcessCommand which already has the lock
// 		myStepper->Stop();

// 		switch (myState)
// 		{
// 		case States::ACCELERATING:
// 		case States::COASTING:
// 			myState = States::DECELERATING;
// 			break;
// 		case States::DECELERATING:
// 			return;
// 			break;
// 		case States::STOPPED:
// 			return;
// 		}
// 	}

// 	template <typename DerivedStepper>
// 	void StepperState<DerivedStepper>::ProcessNewSpeed(uint32_t speed)
// 	{
// 		// No need for mutex here as it's called from within ProcessCommand which already has the lock
// 		auto mySpeed = myStepper->GetCurrentSpeed();
// 		auto myDirection = myStepper->GetDirection();
// 		auto myTargetSpeed = myStepper->GetTargetSpeed();
// 		auto myTargetDirection = myStepper->GetTargetDirection();

// 		switch (myState)
// 		{
// 		case States::ACCELERATING:
// 			if (speed < mySpeed)
// 			{
// 				myState = States::DECELERATING;
// 			}

// 			if (mySpeed != speed)
// 			{
// 				myStepper->SetSpeed(speed);
// 			}
// 			break;
// 		case States::COASTING:
// 			if (speed < mySpeed)
// 			{
// 				myStepper->SetSpeed(speed);
// 				myState = States::DECELERATING;
// 			}
// 			else if (speed > mySpeed)
// 			{
// 				myStepper->SetSpeed(speed);
// 				myState = States::ACCELERATING;
// 			}
// 			else
// 			{
// 				return;
// 			}
// 			break;
// 		case States::DECELERATING:
// 			if (speed > mySpeed)
// 			{
// 				myStepper->SetSpeed(speed);
// 				myState = States::ACCELERATING;
// 			}
// 			else if (speed == mySpeed)
// 			{
// 				myStepper->SetSpeed(speed);
// 				myState = States::COASTING;
// 			}
// 			else
// 			{
// 				myStepper->SetSpeed(speed);
// 			}
// 			break;
// 		case States::STOPPED:
// 			myState = States::ACCELERATING;
// 			myStepper->SetSpeed(speed);
// 			break;
// 		}
// 	}

// 	template <typename DerivedStepper>
// 	void StepperState<DerivedStepper>::ProcessStart(bool direction, uint32_t speed)
// 	{
// 		// No need for mutex here as it's called from within ProcessCommand which already has the lock
// 		auto mySpeed = myStepper->GetCurrentSpeed();
// 		auto myDirection = myStepper->GetDirection();
// 		auto myTargetSpeed = myStepper->GetTargetSpeed();
// 		auto myTargetDirection = myStepper->GetTargetDirection();

// 		// Start can only occur from STOPPED state
// 		if (!myStepper->IsRunning())
// 		{
// 			return;
// 		}

// 		// We're in STOPPED state, so it's safe to set direction and speed
// 		myStepper->SetDirection(direction);

// 		if (speed != 0)
// 		{
// 			myState = States::ACCELERATING;
// 			myStepper->SetSpeed(speed);
// 			myStepper->Start();
// 		}
// 	}

// } // namespace PowerFeed