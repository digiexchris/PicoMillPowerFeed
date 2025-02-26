#pragma once

// Remove self-include to prevent circular inclusion
#include <PIOStepperSpeedController/Stepper.hxx>
#include <cstdint>
#include <pico/stdlib.h>
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
			stepper.IsRunning()
			} -> std::convertible_to<bool>;
	}
	&&std::derived_from<Derived, Stepper<Derived>>;

	template <typename Derived>
	using StepperType = Stepper<Derived>;

	template <typename Derived>
	class Stepper
	{
	public:
		bool GetDirection();
		bool GetTargetDirection();
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

	template <typename Derived>
	void Stepper<Derived>::Stop()
	{
		static_cast<Derived *>(this)->Stop();
	}

	template <typename Derived>
	void Stepper<Derived>::Start()
	{
		static_cast<Derived *>(this)->Start();
	}

	template <typename Derived>
	void Stepper<Derived>::SetSpeed(uint32_t speed)
	{
		static_cast<Derived *>(this)->SetSpeed(speed);
	}

	template <typename Derived>
	void Stepper<Derived>::Init()
	{
		static_cast<Derived *>(this)->Init();
	}

	template <typename Derived>
	uint32_t Stepper<Derived>::GetCurrentSpeed()
	{
		float aSpeed = static_cast<Derived *>(this)->GetCurrentSpeed();
		return static_cast<uint32_t>(aSpeed);
	}

	template <typename Derived>
	uint32_t Stepper<Derived>::GetTargetSpeed()
	{
		float aSpeed = static_cast<Derived *>(this)->GetTargetSpeed();
		return static_cast<uint32_t>(aSpeed);
	}

	template <typename Derived>
	bool Stepper<Derived>::GetDirection()
	{
		return static_cast<Derived *>(this)->GetDirection();
	}

	template <typename Derived>
	bool Stepper<Derived>::GetTargetDirection()
	{
		return static_cast<Derived *>(this)->GetTargetDirection();
	}

	template <typename Derived>
	void Stepper<Derived>::SetDirection(bool aDirection)
	{
		if (IsRunning())
		{
			// this is a panic because StepperState should prevent this situation
			panic("Stepper: Cannot change direction while running\n");
		}

		static_cast<Derived *>(this)->SetDirection(aDirection);
	}

	template <typename Derived>
	bool Stepper<Derived>::IsRunning()
	{
		// TODO Stepper::GetState needs to be atomic/thread safe
		return static_cast<Derived *>(this)->IsRunning();
	}

	template <typename Derived>
	bool Stepper<Derived>::Update()
	{
		return static_cast<Derived *>(this)->Update();
	}

} // namespace PowerFeed