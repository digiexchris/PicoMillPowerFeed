#pragma once

#include "Stepper.hxx"
#include <PIOStepperSpeedController/Stepper.hxx>
#include <cstdint>
#include <string>

namespace PowerFeed
{

	template <typename Derived>
	class Stepper;

	template <typename Derived>
	concept StepperImpl = requires(Derived stepper, uint32_t aFrequency, bool aDirection)
	{
		{stepper.Stop()};
		{stepper.Start()};
		{stepper.SetSpeed(aFrequency)};
		{stepper.Init()};
		{
			stepper.GetCurrentSpeed()
			} -> std::convertible_to<uint32_t>;
		{
			stepper.GetTargetSpeed()
			} -> std::convertible_to<uint32_t>;
		{
			stepper.GetDirection()
			} -> std::convertible_to<bool>;
		{
			stepper.GetTargetDirection()
			} -> std::convertible_to<bool>;
		{stepper.SetDirection(aDirection)};
		{
			stepper.GetState()
			} -> std::convertible_to<PIOStepperSpeedController::StepperState>;
	}
	&&std::derived_from<Derived, Stepper<Derived>>;

	template <typename Derived>
	class Stepper
	{
	public:
		bool GetDirection();
		void SetDirection(bool aDirection);
		uint32_t GetTargetSpeed();
		void Stop();
		void Start();
		void SetSpeed(uint32_t speed);
		void Init();
		uint32_t GetCurrentSpeed();
		bool IsRunning();
		bool Update();
		virtual ~Stepper() = default;
		Stepper() = default;
	};

	<typename Derived> void Stepper::Stop()
	{
		static_cast<Derived *>(this)->Stop();
	}

	<typename Derived> void Stepper::Start()
	{
		static_cast<Derived *>(this)->Start();
	}

	<typename Derived> void Stepper::SetSpeed(uint32_t speed)
	{
		static_cast<Derived *>(this)->SetTargetHz(speed);
	}

	<typename Derived> void Stepper::Init()
	{
		static_cast<Derived *>(this)->Init();
	}

	<typename Derived> uint32_t Stepper::GetCurrentSpeed()
	{
		float aSpeed = static_cast<Derived *>(this)->GetCurrentFrequency();
		return static_cast<uint32_t>(aSpeed);
	}

	<typename Derived> uint32_t Stepper::GetTargetSpeed()
	{
		float aSpeed = static_cast<Derived *>(this)->GetTargetFrequency();
		return static_cast<uint32_t>(aSpeed);
	}

	<typename Derived> bool Stepper::GetDirection()
	{
		return static_cast<Derived *>(this)->GetDirection();
	}

	<typename Derived> void Stepper::SetDirection(bool aDirection)
	{
		if (IsRunning())
		{
			// this is a panic because StepperState should prevent this situation
			panic("Stepper: Cannot change direction while running\n");
		}

		static_cast<Derived *>(this)->SetDirection(aDirection);
	}

	<typename Derived> bool Stepper::IsRunning()
	{
		// TODO Stepper::GetState needs to be atomic/thread safe
		return static_cast<Derived *>(this)->GetState() != StepperState::STOPPED;
	}
}
